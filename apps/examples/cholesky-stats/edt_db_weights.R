# Prints the headers

generate_common_headers <- function(outfile=NULL) {
    if(!is.null(outfile)) {
        sink(outfile, append=TRUE, sink=FALSE)
    } else outfile <- ''

    cat("#ifndef _EDT_DB_WEIGHTS_H_\n", file=outfile, append=FALSE)
    cat("#define _EDT_DB_WEIGHTS_H_\n\n", file=outfile, append=TRUE)
    cat("struct _edtWeightsStruct gEdtWeightsTable[] = {\n\n", file=outfile, append=TRUE)
}

# Prints the footers

generate_common_footers <- function(outfile=NULL) {
    if(!is.null(outfile)) {
        sink(outfile, append=TRUE, sink=FALSE)
    } else outfile <- ''

    cat("\n};\n", file=outfile, append=TRUE)
    cat("#endif // _EDT_DB_WEIGHTS_H_\n\n", file=outfile, append=TRUE)
}


#################################################################

# Collect & filter the input

logfile <- paste(prefix, ".edt", sep="")
exefile <- paste(prefix, ".exe", sep="")

symbols <- system(paste("nm ",exefile, "| grep ' T '", sep=""), intern=T)
symbols <- read.table(textConnection(symbols))
symbols$V1 <- as.numeric(paste("0x", symbols$V1, sep=""))

profile <- read.table(logfile)
profile <- profile[,c(2,3,7,15,19)]
names(profile) <- c("edt", "slot", "size", "reads", "writes")

generate_common_headers()

# Collect unique EDTs

edts <- unique(profile$edt)

for (edt in edts) {
    test <- profile[profile$edt==edt,]

    # Count the number of EDT instances
    edtcounts <- nrow(test[test$slot==256,])

    allzeroes <- TRUE

    for(counts in seq(1,edtcounts)) {
        totbytes <- 0
        for(j in unique(test$slot)) {
            reads <- test[test$slot==j,]$reads[counts]
            writes <- test[test$slot==j,]$writes[counts]
            if(is.na(reads)) reads <- 0
            if(is.na(writes)) writes <- 0
            totbytes <- totbytes + reads + writes
        }
        if(allzeroes && totbytes != 0) allzeroes <- FALSE
        for(j in unique(test$slot)) {
            reads <- test[test$slot==j,]$reads[counts]
            writes <- test[test$slot==j,]$writes[counts]
            if(is.na(reads) || is.na(writes)) next
            if(totbytes != 0) {
                test[test$slot==j,]$reads[counts] <- (reads+writes)/totbytes
            } else test[test$slot==j,]$reads[counts] <- 0
            stopifnot(test[test$slot==j,]$reads[counts] <= 1)
        }
    }

    if(!allzeroes) {
        name <- as.character(symbols$V3[symbols$V1 == edt][1])
        #cat('{"', name, '",',sep="", file=outfile, append=TRUE)
        cat('{"', name, '", ',sep="")

        best_slots <- NULL

        for(j in unique(test$slot)) {
            x <- test[test$slot==j,]$reads
            y <- append(x, vector(length=(edtcounts-length(x))))
            stopifnot(mean(y) <= 1)
            if(mean(y)>0) {
                if(is.na(sd(y)))
                    best_slots <- append(best_slots, c(j, mean(y), 0))
                else
                    best_slots <- append(best_slots, c(j, mean(y), sd(y)))
                #cat("slot ", j, " weight ", mean(y), ' sd ', sd(y),'\n')
            }
        }

        # Now emit the weights
        ans <- data.frame(matrix(best_slots, ncol=3, byrow=TRUE))
        names(ans) <- c("slot", "weight", "sd")
        x <- ans[order(-ans$weight, ans$sd),] # Sort on decreasing weights but increasing sd
        #if(!is.na(x[1,]$weight)) { cat(x[1,]$slot, ', ', round(100*x[1,]$weight), ', ', round(100*x[1,]$sd), ', ', sep="") }
        #coverage <- round(100*x[1,]$weight)
        #if(!is.na(x[2,]$weight)) {
        #    cat(x[2,]$slot, ', ', round(100*x[2,]$weight), ', ', round(100*x[2,]$sd), ', ', sep="")
        #    coverage <- coverage + round(100*x[2,]$weight)
        #} else { cat('0, 0, 0, ') }

        if(!is.na(x[1,]$weight)) {
            s1 <- x[1,]$slot
            w1 <- round(100*x[1,]$weight)
            sd1 <- round(100*x[1,]$sd)
        }
        coverage <- round(100*x[1,]$weight)
        if(!is.na(x[2,]$weight)) {
            s2 <- x[2,]$slot
            w2 <- round(100*x[2,]$weight)
            sd2 <- round(100*x[2,]$sd)
            coverage <- coverage + round(100*x[2,]$weight)
        } else {
            s2 <- 0
            w2 <- 0
            sd2 <- 0
        }

        cat('{ ', s1, ', ', s2, ' }, ', '{ ', w1, ', ', w2, ' }, ', '{ ', sd1, ', ', sd2, ' }, ', sep="")
        cat(coverage, '},\n')

    }
}

generate_common_footers()
