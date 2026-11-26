// The project's pages off GitHub: the community and the ways to support it.
// The code (moved from GitHub, 2026-11-26).
export const GITLAB_URL = 'https://gitlab.com/pokedaisy/pokedaisy';
export const REDDIT_URL = 'https://www.reddit.com/r/PokeDaisy/';
// The invite never expires (Server Settings > Invites); swap it here, in the app (companion/Discord.kt) and in README together.
export const DISCORD_URL = 'https://discord.gg/vcmyKZZV5q';
export const KOFI_URL = 'https://ko-fi.com/pokedaisy';
export const PATREON_URL = 'https://www.patreon.com/cw/pokedaisy';
export const COFFEE_URL = 'https://buymeacoffee.com/lidor30g';

// Obtainium (an Android app that installs and updates apps from where they're published): one tap adds PokeDaisy with
// its HTML source on the newest release's page, the version read from the APK's file name (docs/manual/obtainium.md).
// The config is Obtainium's own app JSON (lib/models/app.dart: additionalSettings a JSON string); the link goes through
// Obtainium's redirect page, which opens the app or offers to get it, so it works from any browser.
const OBTAINIUM_APP = {
  id: 'com.pokedaisy.app',
  url: 'https://pokedaisy.com/releases/latest',
  author: 'PokeDaisy',
  name: 'PokeDaisy',
  overrideSource: 'HTML',
  additionalSettings: JSON.stringify({
    versionExtractionRegEx: 'PokeDaisy-([0-9.]+)\\.apk',
    matchGroupToUse: '1',
    apkFilterRegEx: 'PokeDaisy-[0-9.]+\\.apk',
  }),
};
export const OBTAINIUM_DEEP_LINK = `obtainium://app/${encodeURIComponent(JSON.stringify(OBTAINIUM_APP))}`;
export const OBTAINIUM_URL = `https://apps.obtainium.imranr.dev/redirect?r=${encodeURIComponent(OBTAINIUM_DEEP_LINK)}`;
