#!/usr/bin/Rscript
# Creates an sql script for dropping old tables
# Example usage:
# sqldel.R 2014-01-01 2014-07-01 > /tmp/dropthese.sql
# psql -d dbname -U userWithWriteAccess -a -f /tmp/dropthese.sql

args <- commandArgs(TRUE)
days0 <- seq(as.Date(as.character(args[1])),
             as.Date(as.character(args[2]))-1/2,
             1)
days <- sapply(days0, function(x) format(x,"day%Y%m%d"))
for (d in days) {
  cat("drop table ",d,";\n",sep="")
}
