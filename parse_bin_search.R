library(dplyr)
library(readr)
library(iterators)
library(foreach)


params.all<-foreach(fn=Sys.glob(c("results-*.csv")),.combine = bind_rows) %do%{
	read_csv(fn,col_names = c('items',  'p','keys', 'hedge', 
														'size','successes','trials'),
					 col_types = 'ididiii') 
} %>% arrange(p,items,keys) %>%
	mutate(ci=ifelse(successes<trials,
									 1.96*sqrt(successes/trials*(1-successes/trials)/trials),
									 -(exp(log(.05)/trials)-1)))

params<- params.all %>% 
	group_by(items,p) %>%
	filter(size==min(size)) %>%
	filter(keys==min(keys))


ignore<-foreach(p.thresh=unique(params$p),.combine=bind_rows) %do% {
	filter(params,p==p.thresh) %>% ungroup %>%
		select(items,hedge,keys,size,p) %>% 
		arrange(items) %>%
		write_csv(file.path(paste0('param.export.',p.thresh,".",Sys.Date(),'.csv')))
}
 
