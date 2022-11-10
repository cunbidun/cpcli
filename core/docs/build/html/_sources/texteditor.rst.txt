.. _texteditor:

Sample Text Editor Setup
=========================
This page provides some example key-binding setup for Visual Studio Code and Neovim

Vim/Neovim Setup
-----------------------------
Here is a sample vim key-binding configurations. Make sure to open ``vim`` at root of your workspace folder.
This configuration requires `akinsho/toggleterm.nvim <https://github.com/akinsho/toggleterm.nvim>`_ plugin.

.. code-block:: vim

  tnoremap <Esc> <C-\><C-n>

  function! TermWrapper(command) 
    exec 'wa'
  let l:buf_id = uniq(map(filter(getwininfo(), 'v:val.terminal'), 'v:val.bufnr'))
  if len(l:buf_id) > 0
    exec printf("%sbdelete!", l:buf_id[0]) 
  endif
    exec printf("TermExec direction=vertical cmd='%s'", a:command) 
  endfunction

  command! -nargs=0 Runscript call TermWrapper(printf('cpcli_app --root-dir="%s" --project-config=project_config.json --build', expand('%:p:h')))
  command! -nargs=0 RunWithDebug call TermWrapper(printf('cpcli_app --root-dir="%s" --project-config=project_config.json --build-with-debug', expand('%:p:h')))
  command! -nargs=0 RunWithTerm call TermWrapper(printf('cpcli_app --root-dir="%s" --project-config=project_config.json --build-with-term', expand('%:p:h')))
  command! -nargs=0 TaskConfig call TermWrapper(printf('cpcli_app --root-dir="%s" --project-config=project_config.json --edit-config', expand('%:p:h')))
  command! -nargs=0 ArchiveTask call TermWrapper(printf('cpcli_app --root-dir="%s" --project-config=project_config.json --archive', expand('%:p:h')))
  command! -nargs=0 NewTask call TermWrapper(printf('cpcli_app --new --project-config=project_config.json'))

  " VIM
  autocmd filetype cpp nnoremap <C-M-b> :w <bar> :Runscript<CR>
  autocmd filetype cpp nnoremap <C-M-r> :w <bar> :RunWithTerm<CR>
  autocmd filetype cpp nnoremap <C-M-e> :w <bar> :RunWithDebug<CR>
  autocmd filetype cpp nnoremap <C-M-t> :w <bar> :TaskConfig<CR>
  autocmd filetype cpp nnoremap <C-M-a> :w <bar> :ArchiveTask<CR>
  autocmd filetype cpp nnoremap <C-M-n> :w <bar> :NewTask<CR>


Visual Studio Code Setup
-----------------------------