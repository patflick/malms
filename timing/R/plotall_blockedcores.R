pngheight <- 1080
pngwidth <- 1920

hotpluggedfiles <- dir("../data", "hotpluggedcores*")
loadedfiles <- dir("../data", "loadedcores*")

for (filename in hotpluggedfiles) {
	inputfile <- paste(c("../data/", filename),collapse="")
	filenamePNG <- paste(c(head(unlist(strsplit(filename,NULL)),-3), "png") ,sep="",collapse="")
	png(paste(c("../plots/", filenamePNG),sep="",collapse=""),height=pngheight,width=pngwidth)
	source("blockedcores.R")
	dev.off()
}

for (filename in loadedfiles) {
	inputfile <- paste(c("../data/", filename),collapse="")
	filenamePNG <- paste(c(head(unlist(strsplit(filename,NULL)),-3), "png") ,sep="",collapse="")
	png(paste(c("../plots/", filenamePNG),sep="",collapse=""),height=pngheight,width=pngwidth)
	source("blockedcores.R")
	dev.off()
}
