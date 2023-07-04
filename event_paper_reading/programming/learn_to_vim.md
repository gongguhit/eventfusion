# My_vim_configuration

## Basic keys

Default key settings

Use figure in runoob.com:

![default vim key settings](https://www.runoob.com/wp-content/uploads/2015/10/vi-vim-cheat-sheet-sch1.gif)

## My own Nvim settings
- Key mappings

|key|function|
|----|----|
|'<leader>b'|Buffer|
|'<leader>bt'|Buffer switch|
|'<leader>bc'|Close buffer|
|'<leader>bn'|New tab|
|'<leader>d'|Debugger|
|'<leader>f'|find|
|'<leader>g'|git|
|'H'|Go to end without blank|
|'<leader>j'|jupyter|
|'jk'|Esc|
|'J'|Move line down|
|'K'|Move line up|
|'<leader>l'|lsp|
|'<leader>l'|lsp|
|'<leader>m'|Neotest|
|'<leader>n'|NeoAI|
|'<leader>nm'|marks|
|'<leader>p'|Package|
|'<leader>pd'|Select local/global Neoconf config|
|'<leader>r'|Refactor|
|'<leader>s'|leap|
|'<leader>t'|Trouble|
|'<leader>u'|UI/UX|
|'v'|visual mode|
|'v>'|visual mode Indent to the right|
|'v<'|visual mode Indent to the left|
|'w'|Window|
|'<leader>z'|Zen mode|
|'\|'|Split Window horizontal|
|'\\'|Split Window vertical|
|'['|previous|
|']'|next|

### Nvim Plugins

## Skills

Reference 
1. write a motion
```bash
# vimrc中
# onoremap: o no recursive map;;
# Is: motion name;
# : command mode;
# <c-u> 消除符号;
# normal: normal mode;
# F]: 反向查找到];
# vi]: 选择[]中内容并进入visual mode;
onoremap Is :<c-u>normal! F]vi]<cr>
```
```lua
-- nvim lua
maps.i["jk"] = {"<Esc>", desc = "Exit to normal mode"}
```
2. Regular expressions
```bash
40i#jk
```
########################################
```bash
# in vimrc
vnoremap . :normal! .<CR>
```

```lua
-- in nvim lua
maps.v["."] = {":normal! .<CR>", desc = "visual mode . same to normal mode"}
```
3. Change number

- Change 1 number
Cursor on the number，`ctral a` to increase, `ctrl + x` to decrease
dafdfa 15 dfadfadfa

- change number in multiple rows

Use `ctrl + v` and `g + C-a` or `g + C-x`

array[1] = 0;

array[2] = 0;

array[3] = 0;

array[4] = 0;

array[5] = 0;

array[6] = 0;

array[7] = 0;


- generate numbers

`:put = range(1,10)`

1
2
3
4
5
6
7
8
9
10

`:0put = range(1,10)` : generate at row 0

`for i in range(1,10) | put ='192.168.0.'.i | endfor`

192.168.0.1
192.168.0.2
192.168.0.3
192.168.0.4
192.168.0.5
192.168.0.6
192.168.0.7
192.168.0.8
192.168.0.9
192.168.0.10

4.记录宏

q                     a                    <operation>          q  
--------------------  -------------------- -------------------- --------------------   
record command      record to register a  operation            record finished

`qa<operation>q`

使用宏：
@a
<number>@a

Recursively usage

########################################
q                             a                    <operation>              @a                q 
---------------------  -------------------- -------------------- -------------------- --------------------  
record command         record to register a      operation                recursive        record finished


替代方案

加行号：
`%s/^/\=printf('%-4d', line('.'))`

%s: 替换每一行
/^: 行首
/\=: 转义=
line('.'): 当前行


6. search

Normal mode

模糊匹配
`/` : from top to bottom
`?` : bottom to top

`n/N` to pick up next/previous

`*/#`: 精确搜索
`g# == ?` : 模糊搜索

7. regular expression

[ab1]
[0-9]
[a-z]
[A-Z]

`\v` to transfer general regular expression

[^a-z]: except a-z

.   任意一个字符 单独占位
?   没有或者一个 对前一个字符
*   没有或者多个 对前一个字符
+   一个或者多个 对前一个字符
