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

OBJS=Session.o Server.o Logger.o  \
	 SmtpServer.o Message.o  \
	 Main.o DataModel.o DataList.o \
     PopSession.o PopServer.o

	 
LIBS=-lmysqlclient -lpthread -ldevplus -ldevplus-main -ldevplus-thread -ldevplus-mysql

#CONFIG_DIR="/etc"
#LOG_DIR="/var/log/mailsrvd"

H_Defaults=Defaults.h
H_Logger=Logger.h
H_DataList=DataList.h
H_DataModel=DataModel.h $(H_DataList)
H_Session=Session.h $(H_Logger) $(H_Defaults) $(H_DataModel)
H_Message=Message.h $(H_Session) $(H_Defaults)
H_PopSession=PopSession.h $(H_Session) $(H_Defaults)
H_Server=Server.h $(H_Session) $(H_Logger) $(H_DataModel)
H_PopServer=PopServer.h $(H_Server)
H_SmtpServer=SmtpServer.h $(H_Server)
H_SenderSession=SenderSession.h $(H_Session)
H_Sender=Sender.h $(H_SenderSession)

all: mailsrvd
# mailsrv-smtp mailsrv-out

mailsrvd: $(OBJS)
	g++ -o $@ $(OBJS) $(LIBS)

Main.o: main.cpp $(H_Defaults) $(H_Logger) $(H_Server) $(H_PopServer) $(H_SmtpServer) $(H_DataModel)
	g++ -Wall -c -o $@ main.cpp
	


PopServer.o: PopServer.cpp $(H_Server) $(H_PopSession)
	g++ -Wall -c -o $@ PopServer.cpp

Logger.o: Logger.cpp $(H_Logger) $(H_Defaults)
	g++ -Wall -c -o $@ Logger.cpp

Server.o: Server.cpp $(H_Server)
	g++ -Wall -c -o $@ Server.cpp

SmtpServer.o: SmtpServer.cpp $(H_SmtpServer) $(H_Message)
	g++ -Wall -c -o $@ SmtpServer.cpp

Message.o: Message.cpp $(H_Message) $(H_DataList)
	g++ -Wall -c -o $@ Message.cpp 

Session.o: Session.cpp $(H_Session)
	g++ -Wall -c -o $@ Session.cpp

PopSession.o: PopSession.cpp $(H_PopSession)
	g++ -Wall -c -o $@ PopSession.cpp

SenderSession.o: SenderSession.cpp $(H_SenderSession)
	g++ -Wall -c -o $@ SenderSession.cpp

DataModel.o: DataModel.cpp $(H_DataModel) $(H_Logger)
	g++ -Wall -c -o $@ DataModel.cpp

DataList.o: DataList.cpp $(H_DataList)
	g++ -Wall -c -o $@ DataList.cpp


clean:
	@-rm *.o


