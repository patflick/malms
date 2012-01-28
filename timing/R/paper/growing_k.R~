inputfile <- "../../data/growing_k.csv"
outputfile <- "../../plots/paper/growing_k.png"
outputwidth <- 1200
outputheight <- 800
textsize <- 4

data <- read.csv(inputfile,header=TRUE,sep=";")

trimmedmean <- function(x) {
	return <- mean(x, trim=0.10)
	#return <- mean(x)
}

datamean <- aggregate(data,by=list(data$Cores,data$Workpakets,data$Input.Size),FUN=trimmedmean)

inputsizes <- unique(datamean$Input.Size)
cores <- unique(datamean$Cores)
wps <- unique(datamean$Workpakets)

# create output png
png(outputfile,width=outputwidth,height=outputheight)

# plot running time while cores are blocked
size = 1000000
par(cex=2.0)
plotdata <- datamean[which(datamean$Input.Size == size),]
x <- wps
malms <- plotdata$Time.MALMS
plot(NaN,xlim=c(min(x),max(x)),ylim=c(min(0),max(malms)),xlab="Workpackets k",ylab="Abs Time",main="Running time with growing k")


xx <- c(wps, rev(wps))
yyTotal <- c(malms, rep(0,length(x)))
yySort <- c(plotdata$Time.Sorting, rep(0,length(x)))
yyMerge <- c(plotdata$Time.Merging+plotdata$Time.Sorting, rep(0,length(x)))
yySplit <- c(plotdata$Time.Merging+plotdata$Time.Splitting+plotdata$Time.Sorting, rep(0,length(x)))

polygon(xx,yyTotal, col="grey")
polygon(xx,yySplit, col="#FFA200")
polygon(xx,yyMerge, col="#00BB3F")
polygon(xx,yySort, col="#7309AA")

legend("topleft", title="Phases", c("Init Scheduler","Splitting","Merging", "Sorting"),fill=c("grey","#FFA200","#00BB3F", "#7309AA"))
#lines(x,malms,col="blue",type="o")

dev.off()

