
LUT_FILES += \
        blue_lut.png \
        fire_lut.png \
        hotter_lut.png \
        ice_lut.png \
        six_shades_lut.png \
        sixteen_colors_lut.png \
        deep_red_lut.png \
        deep_blue_lut.png


unix{
        LUTEXPORTDIR = "/home/adt/micro-i/hmi_oct/luts"
}

win32{
    LUTEXPORTDIR = $$shell_path($$DESTDIR/luts)
}

##Create lut directory
exists($$LUTEXPORTDIR){
        message("lutdir existing")
}else{
	unix{
		QMAKE_POST_LINK += $$quote(mkdir -p $${LUTEXPORTDIR} $$escape_expand(\\n\\t))
	}
	win32{
		QMAKE_POST_LINK += $$quote(md $${LUTEXPORTDIR} $$escape_expand(\\n\\t))
	}
}

##Copy lookup table folder to "LUTEXPORTDIR"
for(file, LUT_FILES):QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$shell_path($$PWD/$${file})) $$quote($$LUTEXPORTDIR) $$escape_expand(\\n\\t)

#message($$QMAKE_POST_LINK)

##Add lookup table files to clean directive. When running "make clean" lookup table files will be deleted
#for(file, LUT_FILES):QMAKE_CLEAN += $$shell_path($$LUTEXPORTDIR/$${file}) #todo: this does not work probably because LUT_FILES contains the full paths of the files and not just the file name. Find a nice and easy way to add doc files to QMAKE_CLEAN
