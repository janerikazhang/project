#!/bin/bash
#----------------this are the commands for initial set up of the database and tables-----
#
#
# Author: Rika
# Created on 18 Feb,2013


# # # create a datebase # # #
gcc database_setup.c -o database_setup  `mysql_config --cflags --libs`
./database_setup

# # # create a table # # #
g++ -o table_setupcpp table_setupcpp.cpp `mysql_config --cflags` `mysql_config --libs`
./table_setupcpp

# # # insert data # # #

# # # use command line tools of mysql # # #
mysql -u parallels -p'lolita1168' cgroups
# or in root ---
mysql -u root -p'lolita1168' cgroups

