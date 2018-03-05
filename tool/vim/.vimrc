set number
"echo expand('%:p')
"set ruler
"set cursorcolumn
"set cursorline
set cul
highlight CursorLine   cterm=NONE ctermbg=black ctermfg=green guibg=NONE guifg=NONE
set noruler
"file type detect
filetype plugin indent on

"for file type detect
"filetype plugin indent on
""for taglist configure
let Tlist_Auto_Open=1
let Tlist_Ctags_Cmd = '/usr/bin/ctags'
let Tlist_Show_One_File = 1 "不同时显示多个文件的tag，只显示当前文件的
let Tlist_Exit_OnlyWindow = 1 "如果taglist窗口是最后一个窗口，则退出vim
"let Tlist_Use_Right_Window = 1 "在右侧窗口中显示taglist窗口
"let Tlist_Use_Left_Window = 1 "在右侧窗口中显示taglist窗口
"set tags=tags;
"syntax on
""highlight Normal ctermfg=black ctermbg=86acd9

"set autoindent
"set cindent
"
""设置C/C++语言的具体缩进方式
set cinoptions={0,1s,t0,n-2,p2s,(03s,=.5s,>1s,=1s,:1s
set smartindent
set expandtab     " 使用空格代替tab.
set ts=4 "空格数量
set shiftwidth=4 "自动缩进的宽度
set showmatch

