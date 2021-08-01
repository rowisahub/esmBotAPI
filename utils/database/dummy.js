// dummy (no-op) database handler
const misc = require("../misc.js");
const logger = require("../logger.js");

logger.warn("Using dummy database adapter. If this isn't what you wanted, check your DB variable.");

exports.setup = async () => {};
exports.stop = async () => {};
exports.fixGuild = async () => {};
exports.addCount = async () => {};
exports.getCounts = async () => {
  return {};
};
exports.disableChannel = async () => {};
exports.enableChannel = async () => {};
exports.toggleTags = async () => {};
exports.setTag = async () => {};
exports.removeTag = async () => {};
exports.setPrefix = async () => {};
exports.addGuild = async (guild) => {
  return {
    id: guild.id,
    tags: misc.tagDefaults,
    prefix: process.env.PREFIX,
    disabled: [],
    tagsDisabled: false,
  };
};
exports.getGuild = exports.addGuild;
