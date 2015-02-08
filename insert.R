#!/usr/bin/Rscript
## SETTINGS
## sampling rate
srate <- 87380
## database settings
host <- ""
dbname <- ""
user <- ""
password <- ""
## MAIN
args <- commandArgs(TRUE)
#args <- "00000108.cso"

L <- readLines(args[1],warn=FALSE)
T <- as.integer(unlist(strsplit(L[1],"\\.")))
l <- nchar(T[2])
fr <- as.integer(T[2]*10^(6-l))
L <- L[-1] # reducing line vector
D <- as.integer(sub("sc ","",L[-grep("start|stop",L)])) # actual data content
## Timestamping data
fs <- seq(from=fr,by=srate,length.out=length(D))
sfs <- sapply(fs,function(x) {nch <- nchar(x)
                              c(substr(x,1,nch-6),substr(x,nch-5,nch))})
secs <- as.integer(sfs[1,])
secs[is.na(secs)] <- 0
Nsec1970 <- T[1]+secs 
FracSec <- sfs[2,]
class(FracSec) <- "integer"
class(Nsec1970) <- "integer"
df <- data.frame(ch=D,nsec1970=Nsec1970,fracsec=FracSec)
## Writing to "live" table of database
library(RPostgreSQL)
drv <- dbDriver("PostgreSQL")
con <- dbConnect(drv, host=host,dbname=dbname,
                 user=user,password=password)
dbWriteTable(con,"live",df,append=TRUE,row.names=FALSE)
dbDisconnect(con)
