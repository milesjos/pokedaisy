---
title: Install and update with Obtainium
summary: Let Obtainium install PokeDaisy and keep it up to date, with one tap or by adding it by hand.
section: start
order: 79
keywords: [obtainium, app store, auto update, automatic updates, updater, fdroid, f-droid, droidify, install, sideload, apk, update notification]
---

[Obtainium](https://github.com/ImranR98/Obtainium) is a free Android app that installs apps from where they're published and tells you when there's a new version. PokeDaisy already updates itself, so you don't need it, but if you keep your apps in Obtainium, PokeDaisy can be one of them.

## Add it with one tap

1. Install Obtainium on the device first.
2. On the same device, open [pokedaisy.com/obtainium](https://pokedaisy.com/obtainium), or tap **ADD TO OBTAINIUM** under the Android download on [pokedaisy.com](https://pokedaisy.com/#download-android).
3. Obtainium opens and shows what it's about to add. Confirm it.
4. In Obtainium, tap PokeDaisy and **Install**. If you already have PokeDaisy, it recognises the version you have.

If the link only shows a page asking you to get Obtainium, Obtainium isn't installed on that device yet.

## Add it by hand

1. In Obtainium, tap **Add App**.
2. For the URL, enter `https://pokedaisy.com/releases/latest`.
3. Open **Override source** and pick **HTML**.
4. Set **Version extraction RegEx** to `PokeDaisy-([0-9.]+)\.apk` and **Match group to use** to `1`. This reads the version from the APK's name, so Obtainium shows `1.6.2.2` rather than a code.
5. Tap **Add**, then **Install**.

## Good to know

- Obtainium checks for new versions on its own schedule. You can also pull down on its list to check now.
- It installs the same APK as the website's download, signed the same way, so updates go over the PokeDaisy you have and your games, saves and settings stay.
- Don't use both Obtainium's updates and PokeDaisy's own `UPDATE` at the same moment. Either one works.
