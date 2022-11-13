.. _texteditor:

Sample Text Editor key-binding Setup
====================================
This page provides some example key-binding setup for Visual Studio Code and Neovim

Vim/Neovim Setup
-----------------------------
Here is a sample vim key-binding configurations. Make sure to open ``vim`` at root of your workspace folder.
This configuration requires `akinsho/toggleterm.nvim <https://github.com/akinsho/toggleterm.nvim>`_ plugin.

.. code-block:: vim

  tnoremap <Esc> <C-\><C-n>

  let g:cpTermSize = '100'

  function! TermWrapper(command) 
    exec 'wa'
    let l:buf_id = uniq(map(filter(getwininfo(), 'v:val.terminal'), 'v:val.bufnr'))
    if len(l:buf_id) > 0
      exec printf("%sbdelete!", l:buf_id[0]) 
    endif
    exec printf("TermExec direction=vertical cmd='%s'", a:command) 
  endfunction

  command! -nargs=0 Runscript call    TermWrapper(printf('clear; cpcli_app task --root-dir="%s" --build', expand('%:p:h')))
  command! -nargs=0 RunWithDebug call TermWrapper(printf('clear; cpcli_app task --root-dir="%s" --build-with-debug', expand('%:p:h')))
  command! -nargs=0 RunWithTerm call  TermWrapper(printf('clear; cpcli_app task --root-dir="%s" --build-with-term', expand('%:p:h')))
  command! -nargs=0 TaskConfig call   TermWrapper(printf('clear; cpcli_app task --root-dir="%s" --edit-problem-config', expand('%:p:h')))
  command! -nargs=0 ArchiveTask call  TermWrapper(printf('clear; cpcli_app task --root-dir="%s" --archive', expand('%:p:h')))
  command! -nargs=0 NewTask call      TermWrapper(printf('clear; cpcli_app project --new'))
  command! -nargs=0 DeleteTask call   TermWrapper(printf('mv "%s" ~/.local/share/Trash/files/', expand('%:p:h')))

  autocmd filetype cpp nnoremap <C-M-b> :w <bar> :Runscript<CR>
  autocmd filetype cpp nnoremap <C-M-r> :w <bar> :RunWithTerm<CR>
  autocmd filetype cpp nnoremap <C-M-e> :w <bar> :RunWithDebug<CR>
  autocmd filetype cpp nnoremap <C-M-t> :w <bar> :TaskConfig<CR>
  autocmd filetype cpp nnoremap <C-M-a> :w <bar> :ArchiveTask<CR>
  autocmd filetype cpp nnoremap <C-M-d> :w <bar> :DeleteTask<CR>
  autocmd filetype cpp nnoremap <C-M-n> :w <bar> :NewTask<CR>


Visual Studio Code Setup
-----------------------------

.. code-block:: json

  [
    {
      // cpcli build task
      "key": "ctrl+alt+b",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app task -r \"${fileDirname}\"/ -b\r"
      },
      "when": "resourceExtname == .cpp"
    },
    {
      // cpcli edit task config
      "key": "ctrl+alt+e",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app task -r \"${fileDirname}\"/ -e\r"
      },
      "when": "resourceExtname == .cpp"
    },
    {
      // cpcli run task with terminal
      "key": "ctrl+alt+r",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app task -r \"${fileDirname}\"/ -t\r"
      },
      "when": "resourceExtname == .cpp"
    },
    {
      // cpcli build task with debug flags
      "key": "ctrl+alt+d",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app task -r \"${fileDirname}\"/ -w\r"
      },
      "when": "resourceExtname == .cpp"
    },
    {
      // cpcli archive task
      "key": "ctrl+alt+a",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app task -r \"${fileDirname}\"/ -a\r"
      },
      "when": "resourceExtname == .cpp"
    },
    {
      // cpcli new task
      "key": "ctrl+alt+n",
      "command": "workbench.action.terminal.sendSequence",
      "args": {
        "text": "reset; cpcli_app project -n\r"
      }
    }
  ]
