default : pdf
main=elwperf_2_0

pdf:	
	pdflatex $(main)
	pdflatex $(main)
	-bibtex $(main)
	-bibtex $(main)
	pdflatex $(main)
	pdflatex $(main)

outline :
	pdflatex outline

clean :
	/bin/rm -f *~ *.aux *.log *.bbl *.dvi  *.blg *.lof *lot *.toc $(main).pdf outline.pdf *.out
                                                                                
tidy :
	/bin/rm -f *~ *.aux *.log *.bbl *.dvi *.blg *.lof *lot *.toc *.out
                                                                                
