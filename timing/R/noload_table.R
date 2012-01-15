inputfile <- "../data/noload_wp100.csv"
data <- read.csv(inputfile,header=TRUE,sep=";")

trimmedmean <- function(x) {
	return <- mean(x, trim=0.10)
	#return <- mean(x)
}

datamean <- aggregate(data,by=list(data$Cores,data$Workpakets,data$Input.Size),FUN=trimmedmean)

inputsizes <- unique(datamean$Input.Size)

str <- "Input Size	& MALMS		& MCSTL		& MCSTL Advantage (in percent)	\\\\\n\\hline\n"
for (size in inputsizes) {
	data <- datamean[which(datamean$Input.Size == size),]
	str <- paste(c(str, "$10^",log(size,base=10),"$		& "),collapse="")
	str <- paste(c(str, round(data$Time,digits=6), "	& "),collapse="")
	str <- paste(c(str, round(data$Time.MCSTL,digits=6), "	& "),collapse="")
	
	#str <- paste(c(str, data$Time, "	& "),collapse="")
	#str <- paste(c(str, data$Time.MCSTL, "	& "),collapse="")
	str <- paste(c(str, round(100-data$Time.MCSTL*100/data$Time,digits=1), "\\% \\\\\n"),collapse="")
}
writeLines(str)
