


pngwidth <- 1200
pngheight <- 800

loadedfiles <- dir("../data", "loadedcores*")
 
trimmedmean <- function(x) {
	return <- mean(x, trim=0.10)
	#return <- mean(x)
}
 
for (filename in loadedfiles) {
	inputfile <- paste(c("../data/", filename),collapse="")
	


	
	data <- read.csv(inputfile,header=TRUE,sep=";")



	datamean <- aggregate(data,by=list(data$Cores,data$Blocked.Cores,data$Workpakets,data$Input.Size),FUN=trimmedmean)

	inputsizes <- unique(datamean$Input.Size)
	cores <- unique(datamean$Cores)
	blockedcores <- unique(datamean$Blocked.Cores)
	
	
	for (size in inputsizes) {
	
	
	
	
		filenamePNG <- paste(c(head(unlist(strsplit(filename,NULL)),-4),"_",size,".png") ,sep="",collapse="")
		png(paste(c("../plots/loadedcores/", filenamePNG),sep="",collapse=""),height=pngheight,width=pngwidth)

		par(cex=2.0)
		
		plotdata <- datamean[which(datamean$Input.Size == size & datamean$Cores == 8),]
		x <- blockedcores
		malms <- plotdata$Time
		mcstl <- plotdata$Time.MCSTL
		plot(NaN,xlim=c(min(blockedcores),max(blockedcores)),ylim=c(min(malms,mcstl),max(malms,mcstl)),xlab="# Loaded Cores",ylab="Running Time [s]",main="Loaded cores")
	
		lines(x,malms,type="o",col="blue",pch=1,lty=1)
		lines(x,mcstl,type="o",col="red",pch=4,lty=5)

		perfect_speedup_malms<-plotdata[which(x==max(blockedcores)),]$Time/(max(cores):(max(cores)-max(blockedcores)))
		perfect_speedup_stl<-plotdata[which(x==max(blockedcores)),]$Time.MCSTL/(max(cores):(max(cores)-max(blockedcores)))

		lines(x,perfect_speedup_malms, type="o",col="forestgreen", pch=3, lty=6)

		legend("topleft", title="Algorithms", c("MALMS", "STLMS", "PERFECT"),pch=c(1,4,3),lty=c(1,5,6),col=c("blue","red","forestgreen"))

		dev.off()
	
	}
}
