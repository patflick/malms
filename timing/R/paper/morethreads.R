inputfile <- "../../data/hotpluggedcores_wp48.csv"
inputfile24 <- "../../data/hotpluggedcores_threads24.csv"
inputfile48 <- "../../data/hotpluggedcores_threads48.csv"
inputfile100 <- "../../data/hotpluggedcores_threads100.csv"


size = 10000000
outputfile <- "../../plots/paper/morethreads.png"
outputwidth <- 1200
outputheight <- 800
textsize <- 4
 

data <- read.csv(inputfile,header=TRUE,sep=";")
data24 <- read.csv(inputfile24,header=TRUE,sep=";")
data48 <- read.csv(inputfile48,header=TRUE,sep=";")
data100 <- read.csv(inputfile100,header=TRUE,sep=";")

trimmedmean <- function(x) {
	return <- mean(x, trim=0.10)
	#return <- mean(x)
}

datamean <- aggregate(data,by=list(data$Cores,data$Blocked.Cores,data$Workpakets,data$Input.Size),FUN=trimmedmean)

data24 <- aggregate(data24,by=list(data24$Cores,data24$Blocked.Cores,data24$Workpakets,data24$Input.Size),FUN=trimmedmean)
data48 <- aggregate(data48,by=list(data48$Cores,data48$Blocked.Cores,data48$Workpakets,data48$Input.Size),FUN=trimmedmean)
data100 <- aggregate(data100,by=list(data100$Cores,data100$Blocked.Cores,data100$Workpakets,data100$Input.Size),FUN=trimmedmean)

inputsizes <- unique(datamean$Input.Size)
blockedcores <- unique(datamean$Blocked.Cores)
cores <- unique(datamean$Cores)

# plot running time while cores are blocked

png(outputfile,width=outputwidth,height=outputheight)
par(cex=2.0)
plotdata <- datamean[which(datamean$Input.Size == size & datamean$Cores == 8),]
x <- blockedcores
malms <- plotdata$Time
mcstl8 <- plotdata$Time.MCSTL
mcstl24 <- data24[which(data24$Input.Size == size & data24$Cores == 8),]$Time.MCSTL
mcstl48 <- data48[which(data48$Input.Size == size & data48$Cores == 8),]$Time.MCSTL
mcstl100 <- data100[which(data100$Input.Size == size & data100$Cores == 8),]$Time.MCSTL

plot(NaN,xlim=c(min(blockedcores),max(blockedcores)),ylim=c(min(malms,mcstl8,mcstl100),max(malms,mcstl8,mcstl100)),xlab="# Hotplugged Cores",ylab="Running Time [s]",main="Hotplugged cores")
	
lines(x,malms,type="o",col="blue",pch=1,lty=1)
lines(x,mcstl8,type="o",col="red",pch=4,lty=5)
lines(x,mcstl24,type="o",col="#5A016D",pch=3,lty=6)
lines(x,mcstl48,type="o",col="#A67300",pch=6,lty=3)
lines(x,mcstl100,type="o",col="#188A00",pch=16,lty=2)

perfect_speedup_malms<-plotdata[which(x==max(blockedcores)),]$Time/(max(cores):(max(cores)-max(blockedcores)))
perfect_speedup_stl<-plotdata[which(x==max(blockedcores)),]$Time.MCSTL/(max(cores):(max(cores)-max(blockedcores)))

#lines(x,perfect_speedup_malms, type="o",col="forestgreen", pch=3, lty=6)

legend("topleft", title="Algorithms", c("MALMS", "STLMS 8 Threads", "STLMS 24 Threads","STLMS 48 Threads","STLMS 100 Threads"),pch=c(1,4,3,6,16),lty=c(1,5,6,3,2),col=c("blue","red","#5A016D","#A67300","#188A00"))
dev.off()
