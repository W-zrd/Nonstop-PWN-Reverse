undefined8 UndefinedFunction_0010132f(void)

{
  int iVar1;
  long unaff_RBP;
  long in_FS_OFFSET;
  
  iVar1 = strncmp(*(char **)(*(long *)(unaff_RBP + -0xb0) + 8),"Itz",3);
  if ((((iVar1 == 0) &&
       (iVar1 = strncmp((char *)(*(long *)(*(long *)(unaff_RBP + -0xb0) + 8) + 3),"_0n",3),
       iVar1 == 0)) &&
      (iVar1 = strncmp((char *)(*(long *)(*(long *)(unaff_RBP + -0xb0) + 8) + 6),"Ly_",3),
      iVar1 == 0)) &&
     (iVar1 = strncmp((char *)(*(long *)(*(long *)(unaff_RBP + -0xb0) + 8) + 9),"UD2",3), iVar1 == 0
     )) {
    printf("> HTB{%s}\n",*(undefined8 *)(*(long *)(unaff_RBP + -0xb0) + 8));
  }
  if (*(long *)(unaff_RBP + -8) == *(long *)(in_FS_OFFSET + 0x28)) {
    return 0;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}
