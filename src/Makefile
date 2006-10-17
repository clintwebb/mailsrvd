# //-----------------------------------------------------------------------------
# // Mailsrvd
# //  (c) Copyright Hyper-Active Systems, 2006.
# //
# // Hyper-Active Systems
# // ABN: 98 400 498 123
# // 66 Brown Cr
# // Seville Grove, WA, 6112
# // Australia
# // Phone: +61 434 895 695
# //        0434 895 695
# // contact@hyper-active.com.au
# // 
# //-----------------------------------------------------------------------------
#
# makefile

DEVPLUS=-I. -D_EXCLUDE_DB_MYSQL -D_EXCLUDE_READWRITELOCK

OBJS=DevPlus.o Session.o Server.o Logger.o \
	 SmtpServer.o PopServer.o \
	 Message.o PopSession.o SenderSession.o
LIBS=-lsqlite3 -lpthread

#CONFIG_DIR="/etc"

H_DevPlus=DevPlus.h
H_Defaults=Defaults.h
H_Logger=Logger.h
H_Session=Session.h $(H_DevPlus) $(H_Logger)
H_Message=Message.h $(H_Session)
H_PopSession=PopSession.h $(H_Session)
H_Server=Server.h $(H_DevPlus) $(H_Session) $(H_Logger)
H_PopServer=PopServer.h $(H_DevPlus) $(H_Server)
H_SmtpServer=SmtpServer.h $(H_DevPlus) $(H_Server)
H_SenderSession=SenderSession.h $(H_Session)
H_Sender=Sender.h $(H_DevPlus) $(H_SenderSession)


all: mailsrvd

mailsrvd: main.cpp $(OBJS) $(H_DevPlus) $(H_SmtpServer) $(H_Defaults) $(H_Logger) $(H_PopServer) $(H_SenderSession) 
	g++ -o $@ main.cpp $(OBJS) $(LIBS) $(DEVPLUS) 
# -DCONFIG_DIR=$(CONFIG_DIR)

PopServer.o: PopServer.cpp $(H_Server) $(H_PopSession)
	g++ -Wall -c -o $@ PopServer.cpp $(DEVPLUS)

Logger.o: Logger.cpp $(H_Logger)
	g++ -Wall -c -o $@ Logger.cpp $(DEVPLUS)

Server.o: Server.cpp $(H_Server)
	g++ -Wall -c -o $@ Server.cpp $(DEVPLUS)

SmtpServer.o: SmtpServer.cpp $(H_SmtpServer) $(H_Message)
	g++ -Wall -c -o $@ SmtpServer.cpp $(DEVPLUS)

Message.o: Message.cpp $(H_Message)
	g++ -Wall -c -o $@ Message.cpp $(DEVPLUS)

Session.o: Session.cpp $(H_Session)
	g++ -Wall -c -o $@ Session.cpp $(DEVPLUS)

PopSession.o: PopSession.cpp $(H_PopSession)
	g++ -Wall -c -o $@ PopSession.cpp $(DEVPLUS)

SenderSession.o: SenderSession.cpp $(H_SenderSession)
	g++ -Wall -c -o $@ SenderSession.cpp $(DEVPLUS)




DevPlus.o: DevPlus.cpp DevPlus.h
	g++ -Wall -c -o DevPlus.o DevPlus.cpp $(DEVPLUS)

clean:
	@-rm $(OBJS)


