// Lookup History: shows the dictionary lookups the reader recorded in
// /Lookup History.tsv (and the old archive), with search, a book
// filter, a "not found only" switch, and CSV/TSV downloads.
CrossPoint.registerPlugin(async (container, api) => {
  const FILES = ['/Lookup History old.tsv', '/Lookup History.tsv'];
  const COLUMNS = ['time', 'book', 'author', 'chapter', 'page', 'word', 'headword', 'found', 'dictionary'];

  const el = (tag, props, ...children) => {
    const node = document.createElement(tag);
    Object.assign(node, props || {});
    for (const child of children) node.append(child);
    return node;
  };

  async function readHistory(path) {
    try {
      const r = await fetch('/download?path=' + encodeURIComponent(path), { cache: 'no-store' });
      return r.ok ? await r.text() : '';
    } catch (e) {
      return '';
    }
  }

  // Parse TSV lines into objects; skips header lines (each file starts with one).
  function parse(text) {
    const rows = [];
    for (const raw of text.split('\n')) {
      const line = raw.replace(/\r$/, '');
      if (!line.trim() || line.startsWith('time\t')) continue;
      const f = line.split('\t');
      const row = {};
      COLUMNS.forEach((c, i) => (row[c] = (f[i] || '').trim()));
      if (row.word) rows.push(row);
    }
    return rows;
  }

  const texts = await Promise.all(FILES.map(readHistory));
  const all = texts.flatMap(parse);

  if (!all.length) {
    container.replaceChildren(
      el('h2', { textContent: 'Lookup History' }),
      el('p', {
        textContent:
          'No lookups recorded yet. Look up a word while reading (reader menu > Look Up), then refresh this page. ' +
          'This needs the lookup-history firmware.',
      }),
    );
    return;
  }

  const books = [...new Set(all.map((r) => r.book || '(unknown book)'))].sort((a, b) => a.localeCompare(b));
  const missed = all.filter((r) => r.found !== 'yes').length;
  const uniqueWords = new Set(all.map((r) => r.word.toLowerCase())).size;

  // --- controls ---
  const bookSelect = el('select', {}, el('option', { value: '', textContent: 'All books (' + books.length + ')' }));
  for (const b of books) bookSelect.append(el('option', { value: b, textContent: b }));
  const search = el('input', { type: 'search', placeholder: 'Search words' });
  const missedOnly = el('input', { type: 'checkbox' });
  const view = el('select', {},
    el('option', { value: 'list', textContent: 'Every lookup (newest first)' }),
    el('option', { value: 'words', textContent: 'Words, most looked up first' }));
  const status = el('p');
  const table = el('table', { style: 'width:100%;border-collapse:collapse;font-size:90%;' });

  function filtered() {
    const q = search.value.trim().toLowerCase();
    return all.filter((r) =>
      (!bookSelect.value || (r.book || '(unknown book)') === bookSelect.value) &&
      (!missedOnly.checked || r.found !== 'yes') &&
      (!q || r.word.toLowerCase().includes(q) || r.headword.toLowerCase().includes(q)));
  }

  const cell = (text, tag = 'td') =>
    el(tag, { textContent: text, style: 'border-bottom:1px solid #ccc;padding:3px 6px;text-align:left;vertical-align:top;' });

  function render() {
    const rows = filtered();
    table.replaceChildren();
    if (view.value === 'words') {
      const counts = new Map();
      for (const r of rows) {
        const key = r.word.toLowerCase();
        const entry = counts.get(key) || { word: r.word, n: 0, found: r.found === 'yes', books: new Set() };
        entry.n += 1;
        entry.books.add(r.book || '(unknown book)');
        counts.set(key, entry);
      }
      const sorted = [...counts.values()].sort((a, b) => b.n - a.n || a.word.localeCompare(b.word));
      table.append(el('tr', {}, cell('Word', 'th'), cell('Times', 'th'), cell('Found', 'th'), cell('Books', 'th')));
      for (const e of sorted) {
        table.append(el('tr', {}, cell(e.word), cell(String(e.n)), cell(e.found ? 'yes' : 'no'), cell([...e.books].join(', '))));
      }
      status.textContent = sorted.length + ' different words.';
      return;
    }
    table.append(el('tr', {}, cell('Word', 'th'), cell('Entry', 'th'), cell('Book', 'th'), cell('Where', 'th'), cell('When', 'th')));
    for (const r of rows.slice().reverse()) {
      const where = [r.chapter, r.page ? 'p. ' + r.page : ''].filter(Boolean).join(', ');
      const entry = r.found === 'yes' ? r.headword : 'not found';
      table.append(el('tr', {}, cell(r.word), cell(entry), cell(r.book), cell(where), cell(r.time)));
    }
    status.textContent = rows.length + ' lookups shown.';
  }

  // --- downloads ---
  function download(name, text, type) {
    const url = URL.createObjectURL(new Blob([text], { type }));
    const a = el('a', { href: url, download: name });
    document.body.append(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(url), 1000);
  }
  const csvField = (v) => (/[",\n]/.test(v) ? '"' + v.replace(/"/g, '""') + '"' : v);
  const csvButton = el('button', { type: 'button', textContent: 'Download CSV' });
  csvButton.addEventListener('click', () => {
    const lines = [COLUMNS.join(',')].concat(filtered().map((r) => COLUMNS.map((c) => csvField(r[c])).join(',')));
    // The BOM makes Excel read Thai text as UTF-8; Google Sheets ignores it.
    download('lookup-history.csv', '﻿' + lines.join('\r\n') + '\r\n', 'text/csv;charset=utf-8');
  });
  const tsvButton = el('button', { type: 'button', textContent: 'Download TSV' });
  tsvButton.addEventListener('click', () => {
    const lines = [COLUMNS.join('\t')].concat(filtered().map((r) => COLUMNS.map((c) => r[c]).join('\t')));
    download('lookup-history.tsv', lines.join('\n') + '\n', 'text/tab-separated-values;charset=utf-8');
  });
  const copyButton = el('button', { type: 'button', textContent: 'Copy words' });
  copyButton.addEventListener('click', async () => {
    const words = [...new Set(filtered().map((r) => r.word))].join('\n');
    try {
      await navigator.clipboard.writeText(words);
      status.textContent = 'Copied ' + words.split('\n').length + ' words.';
    } catch (e) {
      status.textContent = 'Copy is not allowed on this page; use Download instead.';
    }
  });

  for (const c of [bookSelect, search, missedOnly, view]) c.addEventListener('input', render);
  container.replaceChildren(
    el('h2', { textContent: 'Lookup History' }),
    el('p', { textContent: all.length + ' lookups, ' + uniqueWords + ' different words, ' + books.length + ' books, ' + missed + ' not found in the dictionary.' }),
    el('div', {}, bookSelect, ' ', view),
    el('div', {}, search, ' ', el('label', {}, missedOnly, ' Not found only')),
    el('div', { style: 'margin:6px 0;' }, csvButton, ' ', tsvButton, ' ', copyButton),
    status,
    table,
  );
  render();
});
