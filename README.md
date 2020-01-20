# Amusement Park Line Simulator

## Introduction
You work as a system engineer team for Universal Studios, which is currently in the process of designing a Jurassic Park ride at the theme park. You are in charge of designing the waiting area, 
God-forsaken land of ropes, where people going around in circles. The ride consists of putting MAXPERCAR or less people on a Ford Explorer and sending through a small-scale version of Jurassic Park where the dinosaurs run loosely and attack tourists.
There are total CARNUM Explorers.
## Specifications
At the beginning of every minute, we can decide the number people who will arrive in this minute by calling poissonRandom(meanArrival), provided in file random437.h.


Assume there is a limit on the number of people kept in the waiting area, MAXWAITPEOPLE(800).


09:00:00--10:59:59, meanArrival = 25 persons per minute

11:00:00--13:59:59, meanArrival = 45 persons per minute

14:00:00--15:59:59, meanArrival = 35 persons per minute

16:00:00--18:59:59, meanArrival = 25 persons per minute


When people arrive, there will a status line “XXX arrive YYY reject ZZZ wait-line WWW at HH:MM:SS” where, XXX is the time step ranging from 0 to 599, YYY is the number of persons arrived, ZZZ is the number of persons rejected, WWW is number of persons in the waiting line, HH:MM:SS is hours, minutes, and seconds

At the end of the day, it will display:
 
The total number of people arrived

The total number of people taking the ride

The total number of people going away due to the long waiting line

The average waiting time per person (in minutes)

Determine the length of the line at its worst case, and the time of day at which that occurs. 
## Usage
-N CARNUM and -M MAXPERCAR
