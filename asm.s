.intel_syntax noprefix
#.globl swtch
swtch:
  mov eax,[esp+4]
  mov edx,[esp+8]
  #movl 4(%esp), %eax
  #movl 8(%esp), %edx

  # Save old callee-save registers
  #pushl %ebp
  #pushl %ebx
  #pushl %esi
  #pushl %edi
  push ebp
  push ebx
  push esi
  push edi

  # Switch stacks
  #movl %esp, (%eax)
  #movl %edx, %esp
  mov [eax], esp
  mov esp, edx

  # Load new callee-save registers
  #popl %edi
  #popl %esi
  #popl %ebx
  #popl %ebp
  pop edi
  pop esi
  pop ebx
  pop ebp
  ret
  