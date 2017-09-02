#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

# Embed file as binary
# extern const uint8_t index_html_start[] asm("_binary_index_html_start");
# extern const uint8_t index_html_end[]   asm("_binary_index_html_end");
COMPONENT_EMBED_FILES := index.html jquery-3.2.1.slim.min.js