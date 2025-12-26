client.o: src/client.c src/file_processor.h src/main.h src/types.h \
 src/client.h
connection.o: src/connection.c src/main.h src/types.h src/ui/app.h \
 src/ui/../common.h src/ui/widget.h src/ui/../main.h \
 src/ui/widget/dialogue.h src/ui/widget/../widget.h \
 src/ui/widget/../../common.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h
file_processor.o: src/file_processor.c src/client.h src/main.h \
 src/types.h src/query.h src/ui/app.h src/ui/../common.h src/ui/widget.h \
 src/ui/../main.h src/ui/widget/dialogue.h src/ui/widget/../widget.h \
 src/ui/widget/../../common.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h \
 src/ui/widget/file_list.h
main.o: src/main.c src/connection.h src/main.h src/types.h src/ui/app.h \
 src/ui/../common.h src/ui/widget.h src/ui/../main.h \
 src/ui/widget/dialogue.h src/ui/widget/../widget.h \
 src/ui/widget/../../common.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h src/query.h \
 src/ui/widget.h src/ui/widget/file_list.h
query.o: src/query.c src/client.h src/main.h src/types.h src/connection.h \
 src/ui/app.h src/ui/../common.h src/ui/widget.h src/ui/../main.h \
 src/ui/widget/dialogue.h src/ui/widget/../widget.h \
 src/ui/widget/../../common.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h src/file_processor.h \
 src/server.h
server.o: src/server.c src/client.h src/main.h src/types.h src/ui.h \
 src/ui/widget/file_list.h src/ui/widget/../widget.h \
 src/ui/widget/../../main.h
ui.o: src/ui.c
