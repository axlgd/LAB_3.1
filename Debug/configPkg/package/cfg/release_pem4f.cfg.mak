# invoke SourceDir generated makefile for release.pem4f
release.pem4f: .libraries,release.pem4f
.libraries,release.pem4f: package/cfg/release_pem4f.xdl
	$(MAKE) -f C:\Users\alang\DOCUME~1\Axel\7MOSEM~1\Arqui\U3\LAB1~1.1\INT2\HVAC_un_hilo_obj\Aux_files/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\alang\DOCUME~1\Axel\7MOSEM~1\Arqui\U3\LAB1~1.1\INT2\HVAC_un_hilo_obj\Aux_files/src/makefile.libs clean

