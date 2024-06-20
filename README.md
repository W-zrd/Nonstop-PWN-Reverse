# Introduction
![alt](/img/background2.jpg)

My daily write-up and notes (Pwn & Reverse Only) while I was preparing to compete in some national CTF events. Writing notes helps me understand better when solving challenges, so I made this repository. All of the challenges here are mostly from public open-source CTF platforms like HackTheBox, pwnable, etc. Make sure you have read the [basic of assembly for CTF here](https://github.com/W-zrd/Learn-Low-Level-Assembly).

> I often push myself to practice Reverse Engineering and Binary Exploitation categories, although during the qualifying round I mostly complete Forensics challenges :')


# Binary Exploitation
## You know 0xDiablos - HackTheBox
Let's  check the binary information and its protection by using `checksec`
```
    Arch:     i386-32-little
    RELRO:    Partial RELRO
    Stack:    No canary found
    NX:       NX unknown - GNU_STACK missing
    PIE:      No PIE (0x8048000)
    Stack:    Executable
    RWX:      Has RWX segments
```
Seems like all the security protection of the file is disabled. Since No Canary found and PIE is disabled, meaning that we can simply overflow the program using basic return2win technique.

First of all, we need to know the offset of the buffer until the overflow occured. Lets set a breakpoint on the `main` function and create an input just to try to overflow the program. I use `cyclic 300` command from `pwndbg` for the making the payload, and here is what i got when debugging with `pwndbg`.

![alt](/img/diablos1.png)

Based on the output above, the Instruction Pointer (`EIP`) is not pointing to an address, instead it's pointing to our input payload. In a computer system, `EIP` is a pointer that is responsible for storing the memory address of the instruction that will be executed next by the processor. If we gain control over the EIP then we can arbitrarily change the program flow.

Since the `EIP` is pointing to the string `waab` or `0x626161677` (in hex), we can also change this value to another memory address, which is where the flag is stored. There are 3 user-defined function in this program:
```c
0x080491e2  flag
0x08049272  vuln
0x080492b1  main
```
Our goal is to get the flag from the `flag` function, which is never called somewhere in the code. But before we go deeper, let's analyze the disassembly of `main` function to understand the program flow. Here is the snippet:

```c
    0x0804930b <main+90> :	call   0x8049070 <puts@plt>
    0x08049310 <main+95> :	add    esp,0x10
==> 0x08049313 <main+98> :	call   0x8049272 <vuln>
    0x08049318 <main+103>:	mov    eax,0x0
    0x0804931d <main+108>:	lea    esp,[ebp-0x8]
```
Here, we have `main+98` which will call the `vuln` function before executing the next instruction. Let's step into that function to analyze further about the program.

```c
    0x0804928a <+24>:	lea    eax,[ebp-0xb8]
    0x08049290 <+30>:	push   eax
==> 0x08049291 <+31>:	call   0x8049040 <gets@plt>
    0x08049296 <+36>:	add    esp,0x10
    0x08049299 <+39>:	sub    esp,0xc
    0x0804929c <+42>:	lea    eax,[ebp-0xb8]
    0x080492a2 <+48>:	push   eax
==> 0x080492a3 <+49>:	call   0x8049070 <puts@plt>
    0x080492a8 <+54>:	add    esp,0x10
    0x080492ab <+57>:	nop
    0x080492ac <+58>:	mov    ebx,DWORD PTR [ebp-0x4]
    0x080492af <+61>:	leave  
    0x080492b0 <+62>:	ret    
```
We found something interesting here. If you look closer, there is a vulnerable part of that function which allows the user to input an arbitrary payload. The `gets` function will continue to store characters past the end of the buffer. So it's dangerous to use the `gets` function because the program will be vulnerable to a buffer overflow attack.

In addition, there is also a puts function that will print the output based on your input. We can use this later to print out the flag.

Since we know that the `gets` function is vulnerable, we can start to exploit the program. Based on our first findings about EIP, the program will overflow when the buffer is greater than 188 characters long. This is proven by the image below.

![alt](/img/diablos2.png)

The next 4 bytes, starting from the 189th character of the payload, will go into the `EIP` register. We can manipulate these 4-byte characters so the `EIP` will point to and execute the memory address we want. This way, we should replace those 4-byte characters with the starting address of the `flag` function, which is `0x080491e2`.

Don't forget about the Byte Ordering of this binary file. The endianness of this file is *Least Significant Bit*, so we must type the address in reverse order like this: `\xe2\x91\x04\x08`. Now let's try our first dummy payload. I use pwntools to do this. Here is the snippet:

```py
p = process('./vuln')
gdb.attach(p, '''
    init-pwndbg
    break *0x080492af
    continue
''')
payload = b'a'*188 + b'\xe2\x91\x04\x08'
p.sendlineafter('0xDiablos:', payload)
```
Set a breakpoint on `0x080492af` so we can analyze the value of the registers (especially `EIP`) just before we hit the return instruction of the `vuln` function

![alt](/img/diablos3.png)

Good. If you look closer at the image above, there is a tiny *arrow down* icon from `vuln+62` to the `flag` address. This means our payload has successfully manipulated the return address to call the `flag` function. But even if you can bypass into the `flag` function, there is another problem. There are 2 comparisons as shown in the image below.

![alt](/img/diablos4.png)

Those comparisons most likely use $AND$ operator so you must pass all the conditions in order to get the flag. The first condition (`0x08049246`) will compare the first argument with `0xdeadbeef`, while the second condition (`0x0804924f`) will compare the second argument with `0xc0ded00d`. If you are still not sure about what's going on, make sure you understand the program flow as shown in the image above.

Before continuing to make the exploit, remember that there is some paddings before we hit the return address. On the x86 architecture, padding generally consists of 8 bytes, whereas on the x64 architecture it consists of 16 bytes. You can check [this article for more details](https://corruptedprotocol.medium.com/common-mistakes-in-basic-stack-buffer-overflow-418c6d62e9d4).

Now let's make the final exploit to pass those conditions. Based on our findings so far, the final exploit will consists of:
- dummy characters to fill 188 buffer
- Memory address of the `flag` function
- 8 bytes padding
- 2x arguments to pass branch conditions (`0xdeadbeef`) and (`0xc0ded00d`) in reverse order (LSB)

![solver](/img/diablos5.png)

Execute the solver script and we got the flag. [Here is the final script](/binary-exploitation/You%20know%200xDiablos/exploit.py) i used to solve this challenge.

# Reverse Engineering
## Behind The Scenes - HackTheBox