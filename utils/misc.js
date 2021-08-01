const util = require("util");
const fs = require("fs");

// random(array) to select a random entry in array
exports.random = (array) => {
  if (!array || array.length < 1) return null;
  return array[Math.floor(Math.random() * array.length)];
};

const optionalReplace = (token) => {
  return token === undefined || token === "" ? "" : (token === "true" || token === "false" ? token : "<redacted>");
};

// clean(text) to clean message of any private info or mentions
exports.clean = async (text) => {
  if (text && text.constructor.name == "Promise")
    text = await text;
  if (typeof text !== "string")
    text = util.inspect(text, { depth: 1 });

  text = text
    .replaceAll("`", `\`${String.fromCharCode(8203)}`)
    .replaceAll("@", `@${String.fromCharCode(8203)}`);

  const { parsed } = require("dotenv").config();
  const imageServers = JSON.parse(fs.readFileSync("./servers.json", { encoding: "utf8" })).image;

  for (const server of imageServers) {
    text = text.replaceAll(server, "<redacted>");
  }

  for (const env of Object.keys(parsed)) {
    text = text.replaceAll(parsed[env], optionalReplace(parsed[env]));
  }

  return text;
};

// regexEscape(string) to escape characters in a string for use in a regex
exports.regexEscape = (string) => {
  return string.replace(/[.*+?^${}()|[\]\\]/g, "\\$&"); // $& means the whole matched string
};

// decodeEntities(string)
exports.decodeEntities = (string) => {
  var translate_re = /&(nbsp|amp|quot|lt|gt);/g;
  var translate = {
    "nbsp": " ",
    "amp": "&",
    "quot": "\"",
    "lt": "<",
    "gt": ">"
  };
  return string.replace(translate_re, function(match, entity) {
    return translate[entity];
  }).replace(/&#(\d+);/gi, function(match, numStr) {
    var num = parseInt(numStr, 10);
    return String.fromCharCode(num);
  });
};

// define defaults for prefixes and tags
exports.defaults = {
  prefix: process.env.PREFIX
};
exports.tagDefaults = {
  help: {
    content: "https://projectlounge.pw/esmBot/help.html",
    author: "198198681982205953"
  }
};
