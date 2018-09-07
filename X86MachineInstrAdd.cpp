#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86InstrInfo.h"
#include "X86RegisterBankInfo.h"
#include "X86RegisterInfo.h"
#include "X86Subtarget.h"
#include "X86TargetMachine.h"
#include "X86FrameLowering.h"
#include "X86MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Analysis/EHPersonalities.h"
#include "llvm/CodeGen/Passes.h" 
#include "llvm/IR/GlobalValue.h"
#include "InstPrinter/X86InstComments.h"
#include "MCTargetDesc/X86BaseInfo.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetOpcodes.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Function.h"

#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/WinEHFuncInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/TargetOptions.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <cstdlib>

#define DEBUG_TYPE "X86-machineinstradd"


using namespace llvm;
using namespace std;

#define X86_MACHINEINSTR_ADD_PASS_NAME "X86 machineinstr add pass"

#define TABLE_BASE 0x200000000000


namespace {
    class X86MachineInstrAdd : public MachineFunctionPass{

    public:
        static char ID;
        uint64_t Globaltime = 0;
        GlobalVariable *GV;

        X86MachineInstrAdd() : MachineFunctionPass(ID){
           // initializeX86MachineInstrAddPass(*PassRegistry::getPassRegistry());
            //filename = "t_writetable.txt";
            //getMemoryStore();
            readKeyTable();
            readUnsafeTable();
            //printWriteTable();
            readSourceFile();
            //printSourceFile();
        }

        bool runOnMachineFunction(MachineFunction &MF) override;

        StringRef getPassName() const override{return X86_MACHINEINSTR_ADD_PASS_NAME;}

        void Print(raw_ostream&);
        // This pass runs after regalloc and doesn't support VReg operands.
        MachineFunctionProperties getRequiredProperties() const override {
            return MachineFunctionProperties().set(MachineFunctionProperties::Property::NoVRegs);
        }

        void getAnalysisUsage(AnalysisUsage &AU) const override {
           AU.setPreservesAll();
    // LiveDebugValues::getAnalysisUsage(AU);
           MachineFunctionPass::getAnalysisUsage(AU);
       }
    private:
  /// Machine instruction info used throughout the class.
    const X86InstrInfo *TII;
    string sourcefilename;
    string keyfilename;
    string unsafefilename;
    string outinputfilename;
    string incallfilename;
    unsigned filelines;
    map<string,string> keywritetable;
    map<string,string> unsafewritetable;
    map<string,string> outinputwritetable;
    map<string,vector<string>> incallwritetable;
    vector<string> sourcefile;

    // void getMemoryStore(){
    //      for(X86MemoryFoldTableEntry Entry : MemoryStoreTable)
    //         outs() << Entry.MemOp << " ";
    // }

    void readKeyTable(){
        ifstream infile;
        infile.open(("./" + keyfilename).data());
        assert(infile.is_open());
        string s;
        while(getline(infile,s)){
            vector<string> v;
            split(s, v, ",");
            deleteAllMark(v[0],"\"");
            deleteAllMark(v[1],"\"");           
            keywritetable.insert(map<string,string>::value_type(v[0]+"!"+v[1],v[1]));
                       
        }
        printKeyTable();
        infile.close();
    }

    void readUnsafeTable(){
        ifstream infile;
        infile.open(("./" + unsafefilename).data());
        assert(infile.is_open());
        string s;
        while(getline(infile,s)){
            vector<string> v;
            split(s, v, ","); 
            deleteAllMark(v[0],"\"");
            deleteAllMark(v[1],"\"");   
            unsafewritetable.insert(map<string,string>::value_type(v[0]+"!"+v[1],v[1]));                        
        }
        printUnsafeTable();
        infile.close();
    }

    void readOutinputTable(){
        ifstream infile;
        infile.open(("./" + outinputfilename).data());
        assert(infile.is_open());
        string s;
        while(getline(infile,s)){
            vector<string> v;
            split(s, v, ",");
            deleteAllMark(v[0],"\"");
            deleteAllMark(v[1],"\"");                                           
            outinputwritetable.insert(map<string,string>::value_type(v[0]+"!"+v[1],v[1]));                        
        }
        printOutinputTable();
        infile.close();
    }

    void readIncallTable(){
        ifstream infile;
        infile.open(("./" + incallfilename).data());
        assert(infile.is_open());
        string s;
        while(getline(infile,s)){
            vector<string> v;
            split(s, v, " ");
            vector<string> lines;
            for(vector<string>::size_type i = 1;i != v.size(); i++)
                lines.push_back(v[i]);           
            incallwritetable.insert(map<string,vector<string>>::value_type(v[0],lines));
                       
        }
       // printKeyTable();
        infile.close();
    }

    void readSourceFile(){
        ifstream infile;
        infile.open(("./" + sourcefilename).data());
        assert(infile.is_open());

        string s;

        while(getline(infile,s)){
            sourcefile.push_back(s);
        }
        infile.close();
    }

    void printSourceFile(){
        vector<string>::iterator iter;
        vector<string>::iterator e = sourcefile.end();
        for(iter = sourcefile.begin();iter != e; iter++){
            errs() << *(iter) << "\n";
        }
    }

    void printKeyTable(){        
        map<string,string>::iterator iter;
        for(iter = keywritetable.begin();iter != keywritetable.end(); iter++){
            vector<string> v;
            split(iter->first, v, "!");
            errs() << v[0]<< ' '<<iter->second<< "\n";            
        }
    }

    void printUnsafeTable(){        
        map<string,string>::iterator iter;
        for(iter = unsafewritetable.begin();iter != unsafewritetable.end(); iter++){
            vector<string> v;
            split(iter->first, v, "!");
            errs() << v[0]<< ' '<<iter->second<< "\n";            
        }
    }

    void printOutinputTable(){        
        map<string,string>::iterator iter;
        for(iter = outinputwritetable.begin();iter != outinputwritetable.end(); iter++){
            vector<string> v;
            split(iter->first, v, "!");
            errs() << v[0]<< ' '<<iter->second<< "\n";            
        }
    }

    void split(const string &s, vector<string> &v, const string &c){
        string::size_type pos1, pos2;
        pos2 = s.find(c);
        pos1 = 0;
        while(string::npos != pos2){
            v.push_back(s.substr(pos1, pos2-pos1));

            pos1 = pos2 + c.size();
            pos2 = s.find(c, pos1);
        }
        if(pos1 != s.length())
            v.push_back(s.substr(pos1));
    }
    void deleteAllMark(string &s, const string &mark){
        string::size_type nsize = mark.size();
        while(1){
            string::size_type pos = s.find(mark);
            if(pos == string::npos)
                return;
            s.erase(pos, nsize);
        }
    }
    string getValueName(const string &s, unsigned col){
        static string str[] = {"[", "]", ";", "*", "+", "-", "/" ,"^" ,"!" ,"&", "|", "%", "=" , "~", "<", ">", "?", ",", "(", ")", "{", "}"};
        static vector<string> op(str, str+22);
        string result;
        string::size_type b = col-1;
        //outs() << s.substr(b,string::npos) << "\n";
        while(string::npos != b){
            if(find(op.begin(), op.end(), s.substr(b,1)) != op.end()){
                //outs() << "found:";
                if(s.substr(b,2) == "++" || s.substr(b,2) == "--" || s.substr(b,2) == "+="){
                    //outs() << "need do something\n";
                    string::size_type t = b -1;
                    while(find(op.begin(), op.end(), s.substr(t,1)) == op.end() && t != 0)
                        t--;
                    string med;
                    if(t != (b-1)){
                        med = s.substr(t+1,b-t-1);
                        deleteAllMark(med," ");
                        deleteAllMark(med,"\t");
                        deleteAllMark(med,"\n");
                    }
                    if(med.empty()){
                        //outs() << "holy shit!\n";
                        t = b +2;
                        while(find(op.begin(), op.end(), s.substr(t,1)) == op.end())
                            t++;
                        result = s.substr(b+2,t-b-2);
                    }
                    else
                        result = med;

                }
                else
                    result = s.substr(col-1, b-col+1);
                return result;
            }
            b++;
        }
    }
    
    int getID(string line,string variable)
    {
        map<string,string>::iterator iter;
        map<string,string>::iterator uiter;
        int keyID=0;int unsafeID=0;int outinputID=0;
       // string s=std::to_string(line);
        for(iter = outinputwritetable.begin();iter != outinputwritetable.end(); iter++)
        {
            vector<string> v;
            split(iter->first, v, "!");
            if(v[0]==line&&iter->second==variable)
                outinputID=4;
        }
        for(iter = keywritetable.begin();iter != keywritetable.end(); iter++)
        {
            vector<string> v;
            split(iter->first, v, "!");
            if(v[0]==line&&iter->second==variable)
                keyID=2;
        }
        for(uiter = unsafewritetable.begin();uiter != unsafewritetable.end(); uiter++)
        {
            vector<string> v;
            split(uiter->first, v, "!");
            if(v[0]==line&&uiter->second==variable)
                unsafeID=1;
        }
        return outinputID+keyID+unsafeID;
    }

    int getID(string variable)
    {
        map<string,string>::iterator iter;
        map<string,string>::iterator uiter;
        int keyID=0;int unsafeID=0;int outinputID=0;
        for(iter = outinputwritetable.begin();iter != outinputwritetable.end(); iter++)
        {
            vector<string> v;
            split(iter->first, v, "!");
            if(iter->second==variable)
                outinputID=4;
        }
        for(iter = keywritetable.begin();iter != keywritetable.end(); iter++)
        {
            vector<string> v;
            split(iter->first, v, "!");
            if(iter->second==variable)
                keyID=2;
        }
        for(uiter = unsafewritetable.begin();uiter != unsafewritetable.end(); uiter++)
        {
            vector<string> v;
            split(uiter->first, v, "!");
            if(uiter->second==variable)
                unsafeID=1;
        }
        return outinputID+keyID+unsafeID;
    }


    vector<unsigned> getCheckID(unsigned line,string variable){

        map<string,string>::iterator iter;
        map<string,string>::iterator uiter;
        map<string,string>::iterator outiter;
        int keyID=0;int unsafeID=0;int outinputID=0;int ID=0;map<string,unsigned> IDs;
        vector<unsigned> newIDs;
        
        for(outiter = outinputwritetable.begin();outiter != outinputwritetable.end(); outiter++)
        {           
            //if(outs<=line&&outiter->second==variable)
            string outs;
            if(outiter->second==variable)
            {
                outinputID=4;
                vector<string> outv;
                split(outiter->first, outv, "!");
                outs=outv[0];
            }
            for(iter = keywritetable.begin();iter != keywritetable.end(); iter++)
            {
                vector<string> v;
                split(iter->first, v, "!");
                string s=v[0];
                if(s==outs&&iter->second==variable)
                //if(iter->second==variable)
                    keyID=2;
            }

            for(uiter = unsafewritetable.begin();uiter != unsafewritetable.end(); uiter++)
            {
                vector<string> uv;
                split(uiter->first, uv, "!");
                string uns=uv[0];
                //unsigned uns=atoi(uv[0].c_str());
                if(uns==outs&&uiter->second==variable)
                //if(uiter->second==variable)
                    unsafeID=1;
            }            
           
            ID=outinputID+keyID+unsafeID;
            if(ID)
                IDs.insert(map<string,unsigned>::value_type(outs,ID));
            keyID=0;unsafeID=0;outinputID=0;
        }
        for(iter = keywritetable.begin();iter != keywritetable.end(); iter++)
        {
            string s;
            //if(s<=line&&iter->second==variable)
            if(iter->second==variable)
            {   
                keyID=2;
                vector<string> v;
                split(iter->first, v, "!");
                s=v[0];
                if(IDs.find(s)!=IDs.end())
                    continue;
            }
            for(uiter = unsafewritetable.begin();uiter != unsafewritetable.end(); uiter++)
            {
                vector<string> uv;
                split(uiter->first, uv, "!");
                string uns=uv[0];
                if(uns==s&&uiter->second==variable)
                    unsafeID=1;
            }
            ID=keyID+unsafeID;
            if(ID)
                IDs.insert(map<string,unsigned>::value_type(s,ID));
            keyID=0;unsafeID=0;            
        }
        for(uiter = unsafewritetable.begin();uiter != unsafewritetable.end(); uiter++)
        {
            vector<string> uv;
            split(uiter->first, uv, "!");
            string uns=uv[0];
            if(uiter->second==variable)
            {   
                unsafeID=1;
                if(IDs.find(uns)!=IDs.end())
                    continue;
                IDs.insert(map<string,unsigned>::value_type(uns,unsafeID));
            }            
            unsafeID=0;
        }
        //IDs.pop_back();
        if(IDs.begin()==IDs.end())
            IDs.insert(map<string,unsigned>::value_type("0",0));
        map<string,unsigned>::iterator iditer;
        for(iditer = IDs.begin();iditer != IDs.end(); iditer++){
            newIDs.push_back(iditer->second);            
        }
        printIDs(newIDs);
        return newIDs;
    }

    vector<unsigned> getIncallID(unsigned line,string variable){
        map<string,vector<string>>::iterator iter;
        vector<unsigned> IDs;
        vector<unsigned> newIDs;
        unsigned ID;
        //for(iter = incallwritetable.begin();iter != ; iter++){
        //    if(iter->first==variable)
        if(incallwritetable.find(variable)==incallwritetable.end())
        {
            newIDs.push_back(0);return newIDs;
        }         
        vector<string> v = incallwritetable[variable];
        for(vector<string>::iterator i = v.begin();i != v.end();i++)
        {    
            IDs=getCheckID(line,*i);
            for(vector<unsigned>::iterator j = IDs.begin(); j != IDs.end(); j++)                           
                newIDs.push_back(*j);
            IDs.clear();
        }                
        return newIDs;
    }

    void printIDs(vector<unsigned> IDs)
    {
        for(vector<unsigned>::iterator i = IDs.begin(); i != IDs.end(); i++){
            outs()<<*i<<"\t";
        }
        outs()<<"\n";
    }

        //检查数组中元素是否是合法写
    void check_array(MachineBasicBlock &MBB, MachineInstr &MI, DebugLoc DL, const X86InstrInfo *TII,unsigned sourcereg, unsigned size,unsigned ID)
    {
        /*
        leaq [address],%rcx
        movq %rcx,%rbx
        movq %rdx,%rax
        shlq $3,%rax
        addq %rax,%rbx
        1:
        movq    %rcx,%r9
	    andq	$536870911, %r9        # imm = 0x1FFFFFFF
	    shlq	$2, %r9
	    movabsq	$35184372088832, %rax   # imm = 0x200000000000
	    addq	%rax, %r9
	    movl	(%r9), %eax
        cmpl $id1,%eax
        je 2f
        cmpl $id2,%eax
        je 2f
        ...
        int $3
        2:
        addq $4,%rcx
        cmpq %rcx,%rbx
        jg 1b
        */

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RCX)
        .addReg(sourcereg);

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RBX)
        .addReg(X86::RCX);

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RAX)
        .addReg(size);

        BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RAX)
        .addReg(X86::RAX)
        .addImm(3);

        BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RBX)
        .addReg(X86::RBX)
        .addReg(X86::RAX);

        StringRef AsmString= "1:"; 
        StringRef  con= "";
        FunctionType* ft=FunctionType::get(Type::getVoidTy(MBB.getParent()->getFunction()->getContext()),false);
        const InlineAsm *IA=InlineAsm::get(ft,AsmString,con,false,false,InlineAsm::AD_ATT);
        unsigned ExtraInfo = 0;
        ExtraInfo |=InlineAsm::AD_ATT;
        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
        .addExternalSymbol(IA->getAsmString().c_str())
        .addImm(ExtraInfo);

        
        BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::R9)
        .addReg(X86::RCX);

        BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32), X86::R9)
        .addReg(X86::R9)
        .addImm(0x1fffffff);

        BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::R9)
        .addReg(X86::R9)
        .addImm(2);

        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
        .addImm(TABLE_BASE);

        BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::R9)
        .addReg(X86::R9)
        .addReg(X86::RAX);

        BuildMI(MBB,MI,DL,TII->get(X86::MOV32rm),X86::EAX)                                     
        .addReg(X86::R9)
        .addImm(1)         
        .addReg(0)                   
        .addImm(0)
        .addReg(0);

        //for(vector<unsigned>::iterator i = line.begin(); i != line.end(); i++){

            BuildMI(MBB,MI,DL,TII->get(X86::CMP32ri))                                     
            .addReg(X86::EAX)
            .addImm(ID);
            StringRef AsmString1= "je 2f";                  
            const InlineAsm *IA1=InlineAsm::get(ft,AsmString1,con,false,false,InlineAsm::AD_ATT);
            BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
            .addExternalSymbol(IA1->getAsmString().c_str())
            .addImm(ExtraInfo);
      //  } 
        StringRef AsmString2= " divq 0\n 2:"; 
        const InlineAsm *IA2=InlineAsm::get(ft,AsmString2,con,false,false,InlineAsm::AD_ATT);
        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
        .addExternalSymbol(IA2->getAsmString().c_str())
        .addImm(ExtraInfo); 

         BuildMI(MBB, MI, DL, TII->get(X86::ADD64ri32),X86::RCX)
        .addReg(X86::RCX)
        .addImm(8);

        BuildMI(MBB, MI, DL, TII->get(X86::CMP64rr))
        .addReg(X86::RBX)
        .addReg(X86::RCX);

        StringRef AsmString3= " jg 1b"; 
        const InlineAsm *IA3=InlineAsm::get(ft,AsmString3,con,false,false,InlineAsm::AD_ATT);
        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
        .addExternalSymbol(IA3->getAsmString().c_str())
        .addImm(ExtraInfo);  

    }
    //将数组所有的RDT表项更新为id
    void update_rdt(MachineBasicBlock &MBB, MachineInstr &MI, DebugLoc DL, const X86InstrInfo *TII, unsigned destreg, unsigned size,unsigned ID){
        /*
        movq %rdi,%rcx
        movq %rcx,%rbx
        movq %rdx,%rax
        shlq %rax,3
        addq %rax,%rbx
        1:
        movq    %rcx,%r9
        andq    0x1fffffff,%r9
        shlq	$2, %r9
	    movabsq	$35184372088832, %rax   # imm = 0x200000000000
	    addq	%rax, %r9
	    movl	$id, (%r9)
        addq $8,%rcx
        cmpq %rcx,%rbx
        jg 1b
        */
        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RCX)
        .addReg(destreg);

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RBX)
        .addReg(X86::RCX);

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RAX)
        .addReg(size);

        BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RAX)
        .addReg(X86::RAX)
        .addImm(3);

        BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RBX)
        .addReg(X86::RBX)
        .addReg(X86::RAX);

        StringRef AsmString= "1:"; 
        StringRef  con= "";
        FunctionType* ft=FunctionType::get(Type::getVoidTy(MBB.getParent()->getFunction()->getContext()),false);
        const InlineAsm *IA=InlineAsm::get(ft,AsmString,con,false,false,InlineAsm::AD_ATT);
        unsigned ExtraInfo = 0;
        ExtraInfo |=InlineAsm::AD_ATT;
        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
        .addExternalSymbol(IA->getAsmString().c_str())
        .addImm(ExtraInfo);

        llvm::BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::R9)
        .addReg(X86::RCX);

        BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32), X86::R9)
        .addReg(X86::R9)
        .addImm(0x1fffffff);

        BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::R9)
        .addReg(X86::R9)
        .addImm(2);

        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
        .addImm(TABLE_BASE);

        BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::R9)
        .addReg(X86::R9)
        .addReg(X86::RAX);

        BuildMI(MBB, MI, DL, TII->get(X86::MOV32mi))
        .addReg(X86::R9)                    
        .addImm(1)
        .addReg(0)
        .addImm(0)         
        .addReg(0)                   
        .addImm(ID);

         BuildMI(MBB, MI, DL, TII->get(X86::ADD64ri32),X86::RCX)
        .addReg(X86::RCX)
        .addImm(8);

        BuildMI(MBB, MI, DL, TII->get(X86::CMP64rr))
        .addReg(X86::RBX)
        .addReg(X86::RCX);

        StringRef AsmString3= " jg 1b"; 
        const InlineAsm *IA3=InlineAsm::get(ft,AsmString3,con,false,false,InlineAsm::AD_ATT);
        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
        .addExternalSymbol(IA3->getAsmString().c_str())
        .addImm(ExtraInfo);

    }
};
char X86MachineInstrAdd::ID = 0;
}

bool X86MachineInstrAdd::runOnMachineFunction(MachineFunction &MF){
    const X86InstrInfo *TII = MF.getSubtarget<X86Subtarget>().getInstrInfo();
    const X86RegisterInfo *TRI = MF.getSubtarget<X86Subtarget>().getRegisterInfo();
    map<int64_t,const DILocalVariable*> VDI;
    map<string,int64_t> FDI;
    MachineRegisterInfo *MRI = &MF.getRegInfo();
    const TargetSubtargetInfo &TSI = MF.getSubtarget();
    const TargetFrameLowering *TFI = TSI.getFrameLowering();
    uint64_t NumBytes = 0;
    MachineBasicBlock &FMBB = MF.front();    
    MachineInstr &FMI = *(FMBB.begin());
    DebugLoc FDL = FMI.getDebugLoc();

    const ValueSymbolTable *VTable=MF.getFunction()->getValueSymbolTable();
   // MF.getFunction()->getValueSymbolTable()->lookup(Name)
    for(const auto &VI :MF.getVariableDbgInfo())
    {
        if (!VI.Var)
           continue;
        unsigned FrameReg = 0;
        int FrameOffset = TFI->getFrameIndexReference(MF, VI.Slot, FrameReg);
        outs()<<VI.Var->getName()<<"  "<<VI.Loc->getLine()<<"  "<<FrameOffset<<"  "<<VI.Slot<<"\n";
        VDI.insert(map<int64_t,const DILocalVariable*>::value_type(FrameOffset,VI.Var));
        FDI.insert(map<string,int64_t>::value_type(VI.Var->getName().str(),FrameOffset));
    }
    /*
    mov     %rsp,%rcx
    andq    %rcx,0x1fffffff
    shlq	$2, %rcx
	movabsq	$35184372088832, %rax   # imm = 0x200000000000
	addq	%rax, %rcx
	movl	$8, (%rcx)
    */
   // outs() << MF.getFunction()->getName() << "\n";
  StringRef fn="main";uint64_t fm = 1;
   if(MF.getFunction()->getName()==fn||MF.getFunction()->getName()=="set_rsp"||MF.getFunction()->getName()=="set_rax"
      ||MF.getFunction()->getName()=="set_rdi"||MF.getFunction()->getName()=="set_rsi"||MF.getFunction()->getName()=="set_rdx"
      ||MF.getFunction()->getName()=="set_rcx"||MF.getFunction()->getName()=="mov_rsi_rdi"||MF.getFunction()->getName()=="set_syscall"
      ||MF.getFunction()->getName()=="execute_all"||MF.getFunction()->getName()=="unused")
   {
       fm=0;
   }
    else{

    BuildMI(FMBB, FMI, FDL, TII->get(X86::MOV64rr), X86::RCX)
    .addReg(X86::RSP);

    BuildMI(FMBB, FMI, FDL, TII->get(X86::AND64ri32), X86::RCX)
    .addReg(X86::RCX)
    .addImm(0x1fffffff);

    BuildMI(FMBB, FMI, FDL,TII->get(X86::SHL64ri),X86::RCX)
    .addReg(X86::RCX)
    .addImm(2); 

    BuildMI(FMBB, FMI, FDL, TII->get(X86::MOV64ri),X86::RAX)                                       
    .addImm(TABLE_BASE);

    BuildMI(FMBB, FMI, FDL, TII->get(X86::ADD64rr),X86::RCX)
    .addReg(X86::RCX)
    .addReg(X86::RAX);  

    BuildMI(FMBB, FMI, FDL, TII->get(X86::MOV32mi))
    .addReg(X86::RCX)                    
    .addImm(1)
    .addReg(0)
    .addImm(0)         
    .addReg(0)                   
    .addImm(8);
    }
    

    for(MachineBasicBlock &MBB : MF)
    {
        //outs() << MBB.getFullName() << "\n";
        const BasicBlock *BB = MBB.getBasicBlock();
        MachineBasicBlock *restoreMBB = MF.CreateMachineBasicBlock(BB);
        for(MachineInstr &MI : MBB)
        {
            int Opcode = MI.getOpcode();
            uint64_t LocalNum = 0;
            int64_t ID=0;
            vector<unsigned> IDs;
            //movabsq
             /*if(Opcode == X86::MOV64ri)
            {
                     if(MI.getOperand(1).isGlobal())
                    {
                        string gvariable="_global_@" + (MI.getOperand(1).getGlobal()->getName()).str();
                        //ID=getID(DL.getLine(),gvariable);
                        outs()<<gvariable<<"\n";
                       // DebugLoc  DL = MBB.findDebugLoc(I);
                        outs() << MI << "\n";
                    }
            } */
            if(DebugLoc  DL = MI.getDebugLoc())
            {
                
                 if(sourcefilename.empty()){ 
                    DL = MI.getDebugLoc();
                    if(DISubprogram *DI = dyn_cast<DISubprogram>(DL.getScope()))
                        sourcefilename = DI->getFilename().str();
                    vector<string> v;
                    split(sourcefilename, v, ".");                   
                    keyfilename = v[0] + "_keywritetable.txt";
                    outs() << "sourcefilename:" << sourcefilename << "\n";
                    outs() << "keyfilename:" << keyfilename << "\n";
                    readKeyTable();
                    readSourceFile();
                    unsafefilename = v[0] + "_unsafewritetable.txt";
                    outs() << "unsafefilename:" << unsafefilename << "\n";                    
                    readUnsafeTable();
                    outinputfilename = v[0] + "_outinputwritetable.txt";
                    outs() << "outinputfilename:" << outinputfilename << "\n";                    
                    readOutinputTable();
                    incallfilename = v[0] + "_pointer.txt";
                    outs() << "incallfilename:" << incallfilename << "\n";                    
                    readIncallTable();
                }
                DL = MI.getDebugLoc();
                if(DISubprogram *DI = dyn_cast<DISubprogram>(DL.getScope()))
                    sourcefilename = DI->getFilename().str();
                vector<string> v;
                split(sourcefilename, v, ".");
                //store
                if(Opcode == X86::MOV8mi || Opcode == X86::MOV8mr || Opcode == X86::MOV16mi || Opcode == X86::MOV16mr
                    || Opcode == X86::MOV32mi || Opcode == X86::MOV32mr || Opcode == X86::MOV64mi32 || Opcode == X86::MOV64mr
                    || Opcode == X86::MOVAPSmr) 
                {
                    /*
                    pushq	%rcx
	                pushq	%rax
                    leaq    [address],%rcx
                    andq    0x1fffffff,%rcx
                    shlq	$2, %rcx
	                movabsq	$35184372088832, %rax   # imm = 0x200000000000
	                addq	%rax, %rcx
	                movl	$id, (%rcx)
	                popq	%rax
	                popq	%rcx
                    */
            
                    if(MI.getOperand(3).isGlobal())
                    {                       
                        string gvariable= "_global_@" + (MI.getOperand(3).getGlobal()->getName()).str();
                        string s=std::to_string(DL.getLine());
                        string line=v[0]+"_"+s;
                        ID=getID(line,gvariable);
                        outs()<<gvariable<<"\t"<<ID<<"\n";
                    
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);
                        
                        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RCX)
                        .addGlobalAddress(MI.getOperand(3).getGlobal(), MI.getOperand(3).getOffset(), MI.getOperand(3).getTargetFlags());
                    }else{

                        vector<string> func;
                        split(MBB.getFullName(), func, ":");
                        if(VDI.find(MI.getOperand(3).getImm()) == VDI.end())
                           continue;
                        const DILocalVariable* localvar=VDI[MI.getOperand(3).getImm()];
                        string localvariable= func[0] + "@" +localvar->getName().str();
                        string s=std::to_string(DL.getLine());
                        string line=v[0]+"_"+s;
                        ID=getID(line,localvariable);
                        outs()<<localvariable<<"\t"<<ID<<"\n";

                        BuildMI(MBB, MI, DL, TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB, MI, DL, TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);

                        BuildMI(MBB, MI, DL, TII->get(X86::LEA64r), X86::RCX)
                        .addReg(MI.getOperand(0).getReg())
                        .addImm(MI.getOperand(1).getImm())
                        .addReg(MI.getOperand(2).getReg())
                        .addImm(MI.getOperand(3).getImm())         
                        .addReg(MI.getOperand(4).getReg());   
                    }            
                                            

                    BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32), X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(0x1fffffff);

                    BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(2);
                    
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(TABLE_BASE);

                    BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RCX)
                    .addReg(X86::RCX)
                    .addReg(X86::RAX);
                            
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV32mi))
                    .addReg(X86::RCX)                    
                    .addImm(1)
                    .addReg(0)
                    .addImm(0)         
                    .addReg(0)                   
                    .addImm(ID);

                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RAX, RegState::Define);  

                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RCX, RegState::Define);
    
                    outs() << MI << "\n";
                }
                //load:movrm,cmpmi,MOVSX
                if(Opcode == X86::MOV64rm || Opcode == X86::MOV32rm || Opcode == X86::MOV8rm ||
                   Opcode == X86::MOV16rm||Opcode == X86::MOVAPSrm){
                    /*
                    pushq	%rcx
	                pushq	%rax
	                leaq    [address],%rcx
	                andq	$536870911, %rcx        # imm = 0x1FFFFFFF
	                shlq	$2, %rcx
	                movabsq	$35184372088832, %rax   # imm = 0x200000000000
	                addq	%rax, %rcx
	                movl	(%rcx), %ecx
                    cmpl $id1,%ecx
                    je 1f
                    cmpl $id2,%ecx
                    je 1f
                    ...
                    "movl $1, %eax\n\t"
			        "movl $0, %edi\n\t"
			        "movl $13, %edx\n\t"
			        "lea s, %rsi\n\t"
			        "syscall"
                    divq 0
                    1:
                    popq	%rax
	                popq	%rcx
                    */
                    string cvariable;
                    if(MI.getOperand(4).isGlobal())
                    {
                       
                        string gvariable= "_global_@" + (MI.getOperand(4).getGlobal()->getName()).str();
                        //string gvariable2= "_Globalfunc_@" + (MI.getOperand(4).getGlobal()->getName()).str();                   
                        outs()<<gvariable<<"\t";
                        IDs=getCheckID(DL.getLine(),gvariable);
                        cvariable=gvariable;
                        
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);                        
                        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RCX)
                        .addGlobalAddress(MI.getOperand(4).getGlobal(), MI.getOperand(4).getOffset(), MI.getOperand(4).getTargetFlags());

                    }else{

                        vector<string> func;
                        split(MBB.getFullName(), func, ":");
                        if(VDI.find(MI.getOperand(4).getImm()) == VDI.end())
                           continue;
                        const DILocalVariable* localvar=VDI[MI.getOperand(4).getImm()];
                        string localvariable=func[0] + "@" + localvar->getName().str();
                        outs()<<localvariable<<"\t";
                        IDs=getCheckID(DL.getLine(),localvariable);
                        cvariable=localvariable;
                        
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);

                        BuildMI(MBB,MI,DL,TII->get(X86::LEA64r),X86::RCX)
                        .addReg(MI.getOperand(1).getReg())
                        .addImm(MI.getOperand(2).getImm())
                        .addReg(MI.getOperand(3).getReg())
                        .addImm(MI.getOperand(4).getImm())         
                        .addReg(MI.getOperand(5).getReg());
                    }

                    const Twine &Tmp = "dfi_s";
                    SmallString<256> TmpData;
                    auto M = const_cast<Module*>(MF.getMMI().getModule());    
                    GlobalValue::VisibilityTypes Visibility = GlobalValue::DefaultVisibility;
                    string dla=std::to_string(DL.getLine());
                    const Twine &stri="vul detect! line "+dla+" overflow "+cvariable+"\n";
                    StringRef Str=stri.toStringRef(TmpData);
                    Constant *StrConstant = ConstantDataArray::getString(MF.getFunction()->getContext(), Str);
                    GV = new GlobalVariable(*M, StrConstant->getType(),true, GlobalValue::ExternalLinkage,StrConstant, Tmp);
                    GV->setVisibility(Visibility);
                    GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
                                     
                    BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32), X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(0x1fffffff);

                    BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(2);

                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(TABLE_BASE);

                    BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RCX)
                    .addReg(X86::RCX)
                    .addReg(X86::RAX);

                    BuildMI(MBB,MI,DL,TII->get(X86::MOV32rm),X86::ECX)                                     
                    .addReg(X86::RCX)
                    .addImm(1)         
                    .addReg(0)                   
                    .addImm(0)
                    .addReg(0);
                    
                   for(vector<unsigned>::iterator i = IDs.begin(); i != IDs.end(); i++){
                        
                        BuildMI(MBB,MI,DL,TII->get(X86::CMP32ri))                                     
                        .addReg(X86::ECX)
                        .addImm((*i));                  
                    
                        StringRef AsmString= "je 1f"; 
                        StringRef  con= "";
                        FunctionType* ft=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                        const InlineAsm *IA=InlineAsm::get(ft,AsmString,con,false,false,InlineAsm::AD_ATT);
                        unsigned ExtraInfo = 0;
                        ExtraInfo |=InlineAsm::AD_ATT;
                        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                        .addExternalSymbol(IA->getAsmString().c_str())
                        .addImm(ExtraInfo);
                    }
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(1);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDI)                                       
                    .addImm(0);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDX)                                       
                    .addImm(54);
                    BuildMI(MBB,MI,DL,TII->get(X86::LEA64r),X86::RSI)
                    .addReg(0)
                    .addImm(1)
                    .addReg(0)
                    .addGlobalAddress(GV)        
                    .addReg(0);

                    StringRef AsmString1= "syscall\n divq 0\n 1:"; 
                    StringRef  con1= "";
                    FunctionType* ft1=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                    const InlineAsm *IA1=InlineAsm::get(ft1,AsmString1,con1,false,false,InlineAsm::AD_ATT);
                    unsigned ExtraInfo1 = 0;
                    ExtraInfo1 |=InlineAsm::AD_ATT;
                    BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                    .addExternalSymbol(IA1->getAsmString().c_str())
                    .addImm(ExtraInfo1);
                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RAX, RegState::Define);
                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RCX, RegState::Define);
                    outs() << MI << "\n";

                } 
                //间接调用 call *%rax
                if(Opcode == X86::CALL64r||Opcode == X86::CALL32r){
                    outs() << MI << "\n";                                       

                }
                //间接调用  call *main.fun
                if(Opcode == X86::CALL64m||Opcode == X86::CALL32m){                    
                    vector<string> func;
                    string loadvalue;
                    string cvariable="";
                    vector<unsigned> lines;
                    if(MI.getOperand(3).isGlobal()){

                        string gvariable="_global_@" + (MI.getOperand(3).getGlobal()->getName()).str();
                        vector<string>::iterator iter;
                        string s=std::to_string(DL.getLine());
                        string line=v[0]+"_"+s;
                        outs()<<gvariable<<"\t";
                        IDs=getIncallID(DL.getLine(),gvariable);
                        cvariable=gvariable;
                                                                  
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);

                        BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RCX)                                       
                        .addGlobalAddress(MI.getOperand(3).getGlobal(), MI.getOperand(3).getOffset(), MI.getOperand(3).getTargetFlags());

                        
                    }
                    const Twine &Tmp = "dfi_s";
                    SmallString<256> TmpData;
                    auto M = const_cast<Module*>(MF.getMMI().getModule());    
                    GlobalValue::VisibilityTypes Visibility = GlobalValue::DefaultVisibility;
                    string dla=std::to_string(DL.getLine());
                    const Twine &stri="vul detect! line "+dla+" overflow "+cvariable+"\n";
                    StringRef Str=stri.toStringRef(TmpData);
                    Constant *StrConstant = ConstantDataArray::getString(MF.getFunction()->getContext(), Str);
                    GV = new GlobalVariable(*M, StrConstant->getType(),true, GlobalValue::ExternalLinkage,StrConstant, Tmp);
                    GV->setVisibility(Visibility);
                    GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);                       
                   
                    BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32), X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(0x1fffffff);

                    BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(2);

                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(TABLE_BASE);

                    BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RCX)
                    .addReg(X86::RCX)
                    .addReg(X86::RAX);

                    BuildMI(MBB,MI,DL,TII->get(X86::MOV32rm),X86::ECX)                                     
                    .addReg(X86::RCX)
                    .addImm(1)         
                    .addReg(0)                   
                    .addImm(0)
                    .addReg(0);

                     for(vector<unsigned>::iterator i = IDs.begin(); i != IDs.end(); i++){
                        
                        BuildMI(MBB,MI,DL,TII->get(X86::CMP32ri))                                     
                        .addReg(X86::ECX)
                        .addImm((*i));                  
                    
                        StringRef AsmString= "je 1f"; 
                        StringRef  con= "";
                        FunctionType* ft=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                        const InlineAsm *IA=InlineAsm::get(ft,AsmString,con,false,false,InlineAsm::AD_ATT);
                        unsigned ExtraInfo = 0;
                        ExtraInfo |=InlineAsm::AD_ATT;
                        BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                        .addExternalSymbol(IA->getAsmString().c_str())
                        .addImm(ExtraInfo);
                    }

                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(1);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDI)                                       
                    .addImm(0);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDX)                                       
                    .addImm(54);
                    BuildMI(MBB,MI,DL,TII->get(X86::LEA64r),X86::RSI)
                    .addReg(0)
                    .addImm(1)
                    .addReg(0)
                    .addGlobalAddress(GV)        
                    .addReg(0);

                    StringRef AsmString1= "syscall\n divq 0\n 1:"; 
                    StringRef  con1= "";
                    FunctionType* ft1=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                    const InlineAsm *IA1=InlineAsm::get(ft1,AsmString1,con1,false,false,InlineAsm::AD_ATT);
                    unsigned ExtraInfo1 = 0;
                    ExtraInfo1 |=InlineAsm::AD_ATT;
                    BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                    .addExternalSymbol(IA1->getAsmString().c_str())
                    .addImm(ExtraInfo1);
                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RAX, RegState::Define);
                    BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                    .addReg(X86::RCX, RegState::Define);
                    outs() << MI << "\n";

                }
                if(Opcode == X86::CALL64pcrel32){
                    
                    outs() << MI << "\n";
                    string funcname = "";
                    if(MI.getOperand(0).isGlobal()){
                        funcname = MI.getOperand(0).getGlobal()->getGlobalIdentifier();
                        outs() << funcname << "\n";
                    }
                    if(MI.getOperand(0).isMCSymbol())
                    {                        
                        //outs() << *(MI.getOperand(0).getMCSymbol()) << "\n";
                        funcname = MI.getOperand(0).getMCSymbol()->getName();
                        outs() << funcname << "\n";
                    }

                    if(funcname == "strncpy" || funcname == "memcpy"||funcname == "memset"){
                        //char *strncpy(char *dest,char *src,size_t n);
                        //void *memcpy(void *dest, const void *src, size_t n);
                        //outs() << MI.getOperand(0).getGlobal()->getGlobalIdentifier() << "\n";
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RBX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::R9, RegState::Undef);

                       // check_array(MBB, MI, DL, TII, X86::RSI, X86::RDX,3);
                        update_rdt(MBB, MI, DL, TII, X86::RDI, X86::RDX,3);

                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::R9, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RCX, RegState::Define);                        
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RBX, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RAX, RegState::Define);
                        
                    } 
                    if(funcname == "read"){
                        //ssize_t read[1]  (int fd, void *buf, size_t count);
                       // outs() << MI.getOperand(0).getGlobal()->getGlobalIdentifier() << "\n";
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RBX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::R9, RegState::Undef);

                        update_rdt(MBB, MI, DL, TII, X86::RSI, X86::RDX,2);

                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::R9, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RCX, RegState::Define);                        
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RBX, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RAX, RegState::Define);
                    }
                    if(funcname == "fgets"){
                        //char *fgets(char *buf, int bufsize, FILE *stream);
                        //outs() << MI.getOperand(0).getGlobal()->getGlobalIdentifier() << "\n";
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RAX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RBX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::RCX, RegState::Undef);
                        BuildMI(MBB,MI,DL,TII->get(X86::PUSH64r))
                        .addReg(X86::R9, RegState::Undef);
                       
                        update_rdt(MBB, MI, DL, TII, X86::RSI, X86::RSI,2);

                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::R9, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RCX, RegState::Define);                        
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RBX, RegState::Define);
                        BuildMI(MBB, MI, DL, TII->get(X86::POP64r))
                        .addReg(X86::RAX, RegState::Define);
                    }
                  //  if(funcname == "free"){
                   
                  // }
                }
                if(Opcode == X86::RETQ&fm==1){
                    /*
                    mov     %rsp,%rcx
                    andq    %rcx,0x1fffffff
                    shlq	$2, %rcx
	                movabsq	$35184372088832, %rax   # imm = 0x200000000000
	                addq	%rax, %rcx
	                movl	(%rcx), %ecx                   
                    cmpl    $8,%ecx
                    je 1f
                    int $3
                    1:
                    */

                    const Twine &Tmp = "dfi_s";
                    SmallString<256> TmpData;
                    auto M = const_cast<Module*>(MF.getMMI().getModule());    
                    GlobalValue::VisibilityTypes Visibility = GlobalValue::DefaultVisibility;
                    string dla=std::to_string(DL.getLine());
                    const Twine &stri="vul detect! line "+dla+"overflow,ret\n";
                    StringRef Str=stri.toStringRef(TmpData);
                    Constant *StrConstant = ConstantDataArray::getString(MF.getFunction()->getContext(), Str);
                    GV = new GlobalVariable(*M, StrConstant->getType(),true, GlobalValue::ExternalLinkage,StrConstant, Tmp);
                    GV->setVisibility(Visibility);
                    GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
                   BuildMI(MBB, MI, DL, TII->get(X86::MOV64rr), X86::RCX)
                    .addReg(X86::RSP);

                    BuildMI(MBB, MI, DL, TII->get(X86::AND64ri32),X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(0x1fffffff);

                    BuildMI(MBB,MI,DL,TII->get(X86::SHL64ri),X86::RCX)
                    .addReg(X86::RCX)
                    .addImm(2);

                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(TABLE_BASE);

                    BuildMI(MBB, MI, DL, TII->get(X86::ADD64rr),X86::RCX)
                    .addReg(X86::RCX)
                    .addReg(X86::RAX);

                    BuildMI(MBB,MI,DL,TII->get(X86::MOV32rm),X86::ECX)                                     
                    .addReg(X86::RCX)
                    .addImm(1)         
                    .addReg(0)                   
                    .addImm(0)
                    .addReg(0);

                    BuildMI(MBB, MI, DL, TII->get(X86::CMP32ri))
                    .addReg(X86::ECX)
                    .addImm(8);

                    StringRef AsmString= "je 1f\n "; 
                    StringRef  con= "";
                    FunctionType* ft=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                    const InlineAsm *IA=InlineAsm::get(ft,AsmString,con,false,false,InlineAsm::AD_ATT);
                    unsigned ExtraInfo = 0;
                    ExtraInfo |=InlineAsm::AD_ATT;
                    BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                    .addExternalSymbol(IA->getAsmString().c_str())
                    .addImm(ExtraInfo);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RAX)                                       
                    .addImm(1);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDI)                                       
                    .addImm(0);
                    BuildMI(MBB, MI, DL, TII->get(X86::MOV64ri),X86::RDX)                                       
                    .addImm(36);
                    BuildMI(MBB,MI,DL,TII->get(X86::LEA64r),X86::RSI)
                    .addReg(0)
                    .addImm(1)
                    .addReg(0)
                    .addGlobalAddress(GV)        
                    .addReg(0);
                    StringRef AsmString1= "syscall\n divq 0\n 1:"; 
                    StringRef  con1= "";
                    FunctionType* ft1=FunctionType::get(Type::getVoidTy(MF.getFunction()->getContext()),false);
                    const InlineAsm *IA1=InlineAsm::get(ft1,AsmString1,con1,false,false,InlineAsm::AD_ATT);
                    unsigned ExtraInfo1 = 0;
                    ExtraInfo1 |=InlineAsm::AD_ATT;
                    BuildMI(MBB, MI, DL, TII->get(TargetOpcode::INLINEASM))
                    .addExternalSymbol(IA1->getAsmString().c_str())
                    .addImm(ExtraInfo1);

                }
                
            }

        }
    }
    return true;
}
//char &llvm::X86MachineInstrAddID = X86MachineInstrAdd::ID;
INITIALIZE_PASS(X86MachineInstrAdd, "x86-machineinstr-add",
    X86_MACHINEINSTR_ADD_PASS_NAME,
    false, false
    )

FunctionPass *llvm::createX86MachineInstrAddPass(){return new X86MachineInstrAdd();}

