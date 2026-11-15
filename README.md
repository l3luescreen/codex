# Personal CrossPoint plugins

A small personal plugin catalog for CrossPoint Plugin Hub. It is not part of the
Community catalog.

Catalog URL (paste into Plugin Hub → Custom catalogs):

    https://raw.githubusercontent.com/l3luescreen/codex/plugins/catalog.json

| Plugin | What it does |
|---|---|
| `lookup-history` | Shows `/Lookup History.tsv` (written by the `lookup-history` firmware branch) on the Files page, with search, a book filter and CSV download. |

## Releasing an update

1. Change the files in the plugin folder and raise `version` in its `manifest.json`.
2. Commit and push the plugin files.
3. In `catalog.json`, set the same `version`, put that commit's hash in `release` (short) and in
   `base` (full). Pinning to a commit keeps installs reproducible.
4. Commit and push `catalog.json`. Plugin Hub then shows **Update**.
