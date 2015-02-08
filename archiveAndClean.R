#!/usr/bin/Rscript
## SETTINGS
## database settings
host <- ""
dbname <- ""
user <- ""
password <- ""
## MAIN
today <- Sys.Date()
##today <- as.Date("2013-10-20")
yesterday <- today - 1
dayBeforeYesterday <- yesterday - 1
day.prev.start <- as.integer(as.POSIXct(yesterday))
day.prev.end <- as.integer(as.POSIXct(today))
day.pprev.start <- as.integer(as.POSIXct(dayBeforeYesterday))
## queries and uploads
library(RPostgreSQL)
drv <- dbDriver("PostgreSQL")
con <- dbConnect(drv, host=host,dbname=dbname,
                 user=user,password=password)
sql <- paste("select * from live where nsec1970 + fracsec/power(10,6) >",
             day.prev.start-1,
             "and nsec1970 + fracsec/power(10,6) <",
             day.prev.end+1,
             "order by nsec1970,fracsec asc")
df <- dbGetQuery(con,sql)
yesterday.format <- format(yesterday,format="%Y%m%d")
tblname <- paste("day",yesterday.format,sep="")
success <- dbWriteTable(con,tblname,df,append=TRUE,row.names=FALSE)
## adding a new entry to status db
status <- data.frame(day=as.integer(yesterday),status=as.integer(success))
dbWriteTable(con,"status",status,append=TRUE,row.names=FALSE)
## removing old entries from live database
##day.pprev.start <- 1377786000 # !!!for testing only!!! 
sql.delete <- paste("delete from live where nsec1970 + fracsec/power(10,6) <",
                    day.pprev.start-1)
print(sql.delete)
dbSendQuery(con,sql.delete)
dbDisconnect(con)
