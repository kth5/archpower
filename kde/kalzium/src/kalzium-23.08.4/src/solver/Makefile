
OCAMLC = ocamlc -g
OCAMLMLI = ocamlc
OCAMLOPT = ocamlopt
OCAMLDEP = ocamldep
INCLUDES = -I +facile
OPTOPT = 
OCAMLLIB := $(shell echo `ocamlc -where`)
FACILELIB = $(OCAMLLIB)/facile


# link to these libraries
LIBADD_CMX = $(OCAMLLIB)/facile/facile.cmxa #$(OCAMLLIB)/str.cmxa
LIBADD_CMO = $(OCAMLLIB)/facile/facile.cma #$(OCAMLLIB)/str.cma

# our sources
SOURCES_CHEM = chemset.ml parser.ml lexer.ml datastruct.ml chem.ml  calc.ml
SOURCES_WRAP = modwrap.c

# the targets
CHEM_CMO = $(SOURCES_CHEM:.ml=.cmo)
CHEM_CMX = $(SOURCES_CHEM:.ml=.cmx)
CHEM_O = $(SOURCES_WRAP:.c=.o)

# default target : the object files for eqchem
all : solver.o

# shows how to build a program that embeds the solver
# in compiled ocaml, bytecode ocaml and compiled program using compiled caml code
test: atestprog.opt btestprog.out testcprog

atestprog.opt: $(CHEM_CMX) main.cmx
	$(OCAMLOPT) -o $@ $(LIBADD_CMX) $(CHEM_CMX) main.cmx

btestprog.out: $(CHEM_CMO) main.cmo
	$(OCAMLMLI) -o $@ $(LIBADD_CMO) $(CHEM_CMO) main.cmo

solver.o: $(CHEM_CMX) $(CHEM_O)
	$(OCAMLOPT) -output-obj -o solver.o $(LIBADD_CMX) $(CHEM_CMX)
	cp -f solver.o modwrap.o ../

testcprog: $(CHEM_CMX) $(CHEM_O)
	$(OCAMLOPT) -output-obj -o solver.o $(LIBADD_CMX) $(CHEM_CMX) modwrap.o
	gcc -c -o main.o main.c -L $(OCAMLLIB)
	gcc -o $@ main.o solver.o -L $(OCAMLLIB) \
          -L $(OCAMLLIB)/facile -lasmrun -lstr -lnums -lm -ldl modwrap.o

.SUFFIXES:
.SUFFIXES: .ml .mli .mly .mll .cmi .cmo .cmx .p.cmx .c

.ml.cmo :
	$(OCAMLC) $(INCLUDES) $(INCLUDES) -c $<

.mli.cmi :
	$(OCAMLMLI) $(INCLUDES) -c $<

.ml.cmx :
	$(OCAMLOPT) $(OPTOPT) $(INCLUDES) -c $<

.mly.ml :
	ocamlyacc $<

.mll.ml :
	ocamllex $<

%.o : %.c
	$(OCAMLOPT) $(OPTOPT) $(INCLUDES) -c $<

clean:
	\rm -f *.cm[iox] *.o *~ .depend *testprog.* testcprog

cleanall : clean
	\rm -f *.cmxa *.a *.cma

distclean : clean

interfaces:
	ocamlc -i datastruct.ml > datastruct.mli
	ocamlc -I +facile -i chem.ml > chem.mli
	ocamlc -I +facile -i calc.ml > calc.mli

maintainer-clean : clean

uninstall : clean

install : 
	echo "done"

.depend: parser.ml lexer.ml
	$(OCAMLDEP) $(INCLUDES) *.mli *.ml > $@

include .depend
