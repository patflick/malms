inputfile <- "../../data/hotpluggedcores_wp48.csv"

outputfile <- "../../plots/paper/hotpluggedcores.png"
outputwidth <- 1200
outputheight <- 800
textsize <- 4
 

data <- read.csv(inputfile,header=TRUE,sep=";")

trimmedmean <- function(x) {
	return <- mean(x, trim=0.10)
	#return <- mean(x)
}

datamean <- aggregate(data,by=list(data$Cores,data$Blocked.Cores,data$Workpakets,data$Input.Size),FUN=trimmedmean)

inputsizes <- unique(datamean$Input.Size)
cores <- unique(datamean$Cores)
blockedcores <- unique(datamean$Blocked.Cores)


# plot running time while cores are blocked
size = 1000000
png(outputfile,width=outputwidth,height=outputheight)
par(cex=2.0)
plotdata <- datamean[which(datamean$Input.Size == size & datamean$Cores == 8),]
x <- blockedcores
malms <- plotdata$Time
mcstl <- plotdata$Time.MCSTL
plot(NaN,xlim=c(min(blockedcores),max(blockedcores)),ylim=c(min(malms,mcstl),max(malms,mcstl)),xlab="# Hotplugged Cores",ylab="Abs Time",main="Hotplugged cores")
	
lines(x,malms,type="o",col="blue",pch=1,lty=1)
lines(x,mcstl,type="o",col="red",pch=4,lty=5)

perfect_speedup_malms<-plotdata[which(x==max(blockedcores)),]$Time/(max(cores):(max(cores)-max(blockedcores)))
perfect_speedup_stl<-plotdata[which(x==max(blockedcores)),]$Time.MCSTL/(max(cores):(max(cores)-max(blockedcores)))

lines(x,perfect_speedup_malms, type="o",col="forestgreen", pch=3, lty=6)

legend("topleft", title="Algorithms", c("MALMS", "STLMS", "PERFECT"),pch=c(1,4,3),lty=c(1,5,6),col=c("blue","red","forestgreen"))
dev.off()
