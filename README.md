# yyxLLVM
使用MIR对代码插桩
1、将X86MachineInstrAdd.cpp文件移动到源码llvm/lib/Target/X86文件夹，并添加注册到CMakeLists.txt
2、将FunctionPass *createX86MachineInstrAddPass();添加到X86.h中
3、添加到X86TargetMachine.cpp中：
extern "C" void LLVMInitializeX86Target(){
······
initializeX86MachineInstrAddPass(PR);
}
·······
void X86PassConfig::addPreEmitPass(){
······
addPass(createX86MachineInstrAddPass());
}
4、在llvm/include/llvm/InitializePasses.h中添加void initializeX86MachineInstrAddPass(PassRegistry&);
make
make install
至此注册了一个新的LLVM Pass并且将其添加到最后生成汇编文件之前的最后一个运行的Pass
