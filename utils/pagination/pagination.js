const ReactionCollector = require("./awaitreactions.js");
const MessageCollector = require("./awaitmessages.js");

module.exports = async (client, message, pages, timeout = 120000) => {
  const manageMessages = message.channel.guild && message.channel.permissionsOf(client.user.id).has("manageMessages") ? true : false;
  const options = {
    messageReference: {
      channelID: message.channel.id,
      messageID: message.id,
      guildID: message.channel.guild ? message.channel.guild.id : undefined,
      failIfNotExists: false
    },
    allowedMentions: {
      repliedUser: false
    }
  };
  let page = 0;
  let currentPage = await client.createMessage(message.channel.id, Object.assign(pages[page], options));
  const emojiList = ["◀", "🔢", "▶", "🗑"];
  for (const emoji of emojiList) {
    await currentPage.addReaction(emoji);
  }
  const reactionCollector = new ReactionCollector(client, currentPage, (message, reaction, member) => emojiList.includes(reaction.name) && !member.bot, { time: timeout });
  reactionCollector.on("reaction", async (msg, reaction, member) => {
    if (member.id === message.author.id) {
      switch (reaction.name) {
        case "◀":
          page = page > 0 ? --page : pages.length - 1;
          currentPage = await currentPage.edit(Object.assign(pages[page], options));
          if (manageMessages) msg.removeReaction("◀", member.id);
          break;
        case "🔢":
          client.createMessage(message.channel.id, Object.assign({ content: "What page do you want to jump to?" }, {
            messageReference: {
              channelID: currentPage.channel.id,
              messageID: currentPage.id,
              guildID: currentPage.channel.guild ? currentPage.channel.guild.id : undefined,
              failIfNotExists: false
            },
            allowedMentions: {
              repliedUser: false
            }
          })).then(askMessage => {
            const messageCollector = new MessageCollector(client, askMessage.channel, (response) => response.author.id === message.author.id && !isNaN(response.content) && Number(response.content) <= pages.length && Number(response.content) > 0, {
              time: timeout,
              maxMatches: 1
            });
            return messageCollector.on("message", async (response) => {
              if (await client.getMessage(askMessage.channel.id, askMessage.id).catch(() => undefined)) askMessage.delete();
              if (manageMessages) await response.delete();
              page = Number(response.content) - 1;
              currentPage = await currentPage.edit(Object.assign(pages[page], options));
              if (manageMessages) msg.removeReaction("🔢", member.id);
            });
          }).catch(error => {
            throw error;
          });
          break;
        case "▶":
          page = page + 1 < pages.length ? ++page : 0;
          currentPage = await currentPage.edit(Object.assign(pages[page], options));
          if (manageMessages) msg.removeReaction("▶", member.id);
          break;
        case "🗑":
          reactionCollector.emit("end");
          if (await client.getMessage(currentPage.channel.id, currentPage.id).catch(() => undefined)) await currentPage.delete();
          return;
        default:
          break;
      }
    }
  });
  reactionCollector.once("end", async () => {
    try {
      await client.getMessage(currentPage.channel.id, currentPage.id);
      if (manageMessages) {
        await currentPage.removeReactions();
      }
    } catch {
      return;
    }
  });
  return currentPage;
};
