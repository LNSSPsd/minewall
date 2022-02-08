const fs=require("fs");
const childProcess=require("child_process");
let symbols=[];
let maybedefined=[];
let foundstub=false;
let stubpath;
for(let fn of process.argv[2].split(";")){
	if(fn.indexOf("/stub.c.o")!=-1) {
		foundstub=true;
		stubpath=fn;
		continue;
	}
	console.log("Processing %s",fn);
	let asm=fs.readFileSync(fn).toString();
	if(asm.substr(1,3)=="ELF"){
		console.log("File %s is already compiled.",fn);
		continue;
	}
	//let plttable=asm.match
	//leaq .L.str.id(%rip), %rdi
	//callq xxx@PLT
	//let addrsigs=asm.split(".addrsig\n");
	//let importedsymbols=[];
	asm.replace(/\n.*?:/g,(m)=>{
		let i=m.substr(1).substr(0,m.length-2);
		if(maybedefined.indexOf(i)==-1)maybedefined.push(i);
	});
	asm=asm.replace(/callq.*?\@PLT/g,(mt)=>{
		let ret=mt.replace(/\@PLT/g,"");
		let mr=ret.replace(/callq./g,"");
		if(symbols.indexOf(mr)==-1)symbols.push(mr);
		return mt;
		if(asm.indexOf(".text."+mr)!=-1)return mt;
		if(ret.indexOf("cxa")!=-1||ret.indexOf("cxx")!=-1||ret.indexOf("modloader")!=-1)return mt;
		ret=ret.replace(/callq./g,"callq caller_");
		if(nondd.indexOf(mr)==-1){
			nondd.push(mr);
		}else{
			return ret;
		}
		if(importedsymbols.indexOf(mr)==-1)importedsymbols.push(mr);
		return ret;
	});
	//console.log(importedsymbols);
	/*asm+="\n.addrsig_sym modloader_dlsym_print_error\n";
	let back="";
	let roback="";
	let anotherexecpart=".text\n.section .text.startup,\"ax\",@progbits\n";
	let fi=fn.split("/");
	fi=fi[fi.length-1].replace(/\./g,"_");
	back+=`.type symbols_inited_${fi},@object\n#.bss\n.globl symbols_inited_${fi}\n.p2align 3\nsymbols_inited_${fi}:\n.byte 0\n.size symbold_inited_${fi},1\n\n`;
	let initpart=`.text\n.section .text.startup,"ax",@progbits\n.globl mod_symbolsresolver${fi}\n.type mod_symbolsresolver${fi},@function\nmod_symbolsresolver${fi}:
pushq %rbp\nmovq symbols_inited_${fi}@GOTPCREL(%rip), %rax\nmovb $1, (%rax)\n`;
	for(let i of importedsymbols){
		//console.log(i);
		back+=`.type _addr_${i},@object\n#.bss\n.globl _addr_${i}\n.p2align 3\n_addr_${i}:\n.quad 0\n.size _addr_${i},8\n\n`;
		roback+=`.L.str.iden.${i}:\n.asciz "${i}"\n.size .L.str.iden.${i},${i.length}\n\n`;
		anotherexecpart+=`.globl caller_${i}\n.type caller_${i},@function\ncaller_${i}:\npushq %rbp\nmovq symbols_inited_${fi}@GOTPCREL(%rip), %rax\nmovsbl (%rax),%eax\ncmpl $0,%eax\njne .caller_nseg_of_${i}\npushq %rdi;pushq %rsi;pushq %rdx;pushq %rcx;pushq %r8;pushq %r9\ncallq mod_symbolsresolver${fi}\npopq %r9;popq %r8;popq %rcx;popq %rdx;popq %rsi;popq %rdi\n.caller_nseg_of_${i}:\n#pushq %rdi;pushq %rsi;pushq %rdx;pushq %rcx;pushq %r8;pushq %r9\n#mov .L.str.iden.${i}@GOTPCREL(%rip),%rdi;callq printn@PLT\n#popq %r9;popq %r8;popq %rcx;popq %rdx;popq %rsi;popq %rdi\nmovq _addr_${i}@GOTPCREL(%rip), %rax\n#movq %rsp,%rbp\n#movq %rax,-8(%rbp)\n#movb $0,%al\n#movq -8(%rbp), %rax\ncallq *(%rax)\npopq %rbp\nretq\n`;
		initpart+=`\nmov .L.str.iden.${i}@GOTPCREL(%rip),%rax\nmovq %rax,%rdi\ncallq modloader_dlsym_print_error@PLT\nmovq _addr_${i}@GOTPCREL(%rip), %rbx\nmovq %rax,(%rbx)\n`;
	}
	initpart+="popq %rbp\nretq\n";
	//asm+="\n\n.section .data.codeappend,\"awx\"\n\n";
	asm+=initpart;
	asm+=roback;
	asm+="\n\n.data\n.section .data.codeappended,\"aw\"\n\n";
	asm+=back;
	asm+=anotherexecpart;*/
	fs.writeFileSync(fn+".s",asm);
	childProcess.execSync(`clang++ ${fn}.s -c -fPIC -o ${fn}`);
}

if(!foundstub) {
	console.error("stub.c not created!");
	process.exit(1);
}
for(let i of maybedefined) {
	let l=symbols.indexOf(i);
	if(l==-1)continue;
	symbols.splice(l,1);
}
console.log(symbols);
let bridge=`
extern void *modloader_dlsym_print_error(const char *sym);
static void __mod_init(void);
`;

/*
static void* (*__ptr__bds_func)();
void *bds_func() {
	return __ptr__bds_func();
}
__attribute__((constructor)) static void init() {
	__ptr__bds_func=modloader_dlsym_print_error("bds_func");
}
*/
let initpart="__attribute__((constructor)) static void __mod_init() {\n";

function makeArgList(count) {
	let out="";
	if(count>=1)out="void *a0";
	for(let i=1;i<count;i++) {
		out+=", void *a"+i;
	}
	return out;
}

function makeArgPassList(count) {
	let out="";
	if(count>=1)out="a0";
	for(let i=1;i<count;i++) {
		out+=", a"+i;
	}
	return out;
}

for(let i of symbols) {
	if(i=="modloader_dlsym_print_error")continue;
	if(i.indexOf("modloader")!=-1/*||i=="_ZSt9terminatev"||i.indexOf("_ZNKSt")!=-1||i.indexOf("_ZNSt")!=-1||i=="_ZdlPv"||i=="_Znwm"||i=="memmove"||i=="memcpy"||i.indexOf("cxa")!=-1||i=="memset"||i=="_Unwind_Resume"*/) {
		continue;
	}
	bridge+=`static void* (*__ptr__${i})(${makeArgList(0)})=(void*)0;\nvoid *${i}(${makeArgList(0)}) {\n\t__asm__ __volatile__("pop %% rbp\\njmp *%0"::"m"(__ptr__${i}));\n\treturn __ptr__${i}(${makeArgPassList(0)});\n}\n`;
	initpart+=`\t__ptr__${i}=modloader_dlsym_print_error("${i}");\n`;
}
bridge+=`\n${initpart}}`;
fs.writeFileSync(`${stubpath}.c`,bridge);
childProcess.execSync(`clang ${stubpath}.c -c -fPIC -o ${stubpath}`);
process.exit(0);