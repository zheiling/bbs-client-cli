client.o: src/client.c src/file_processor.h src/main.h src/types.h \
 src/client.h
connection.o: src/connection.c src/main.h src/types.h
file_processor.o: src/file_processor.c src/client.h src/main.h \
 src/types.h src/query.h
main.o: src/main.c src/ui/app.h src/ui/widget/dialogue.h \
 src/ui/widget/../widget.h src/ui/widget/../../main.h \
 src/ui/widget/../../types.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h \
 src/ui/widget/dialogue.h src/ui/modals/login.h \
 src/ui/modals/../widget/dialogue.h
modal.o: src/modal.c src/ui/app.h src/ui/widget/dialogue.h \
 src/ui/widget/../widget.h src/ui/widget/../../main.h \
 src/ui/widget/../../types.h src/ui/widget/button.h \
 src/ui/widget/../../main.h src/ui/widget/group.h
query.o: src/query.c src/file_processor.h src/main.h src/types.h \
 src/connection.h src/client.h src/server.h
server.o: src/server.c src/client.h src/main.h src/types.h \
 src/connection.h src/query.h src/ui.h
ui.o: src/ui.c
