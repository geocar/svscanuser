# normally you'll call this with make args...
# make common/lock.c
all: svscanuser
svscanuser: unix/svscanuser.c common/lock.c
