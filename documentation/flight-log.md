# Flight log

BayuPandu writes every flight to the microSD card as an IGC file, the format
XContest, Leonardo, SeeYou and most flight viewers read. There is nothing to
start or stop.

## How a flight is recorded

- **On the ground** the logger keeps the last 60 seconds of GPS fixes in memory
  and writes nothing.
- **Takeoff** is 5 seconds in a row above 4 m/s (14 km/h) ground speed. The file
  opens with the 60 seconds kept before it, so the launch run is in the log.
- **In the air** one record a second goes straight to the card. Each write
  closes the file again, so if the battery dies only the last second is lost.
  If the GPS loses its fix, records go on at the last position marked `V`
  (no fix), with the pressure altitude still logged.
- **Landing** is 60 seconds in a row below 1.5 m/s ground speed with the vario
  between −0.5 and +0.5 m/s. Hovering in ridge lift does not count: the vario
  shows the climb. The file is then complete.

The Status page shows the logger: **READY**, **REC 12:34** while recording,
**NO CARD**, or **WRITE FAILED** if the card stops taking writes in flight.
The boot screen's **CARD** row shows whether a card was found.

## Files

Logs go in `/flights` on the card, named the IGC way:

```
/flights/2026-10-16-XBP-A1Z-01.igc
          date       │   │   └ flight of the day
                     │   └ this device's id (from its chip)
                     └ maker code (X = not IGC-approved)
```

Times are UTC from the GPS. Pressure altitude is ISA (1013.25 hPa), as the IGC
format asks; GPS altitude is above mean sea level.

## Pilot and glider

Put a `pilot.txt` in the card's top folder, edited on any computer:

```
# BayuPandu pilot details, read at power-on
PILOT=Your Name
GLIDER=Ozone Rush 6
GLIDER_ID=PK-123
```

Keys can be any case; `#` starts a comment; values are cut to 40 characters.
Without the file the header lines are written empty, and you can fill them
in on the league's website when you upload.

## Not IGC-approved

The logs have no G record (security signature), so they are fine for XContest
and club leagues that accept unsigned logs, but not for FAI badges or records.

## In the simulator

The simulator's card is a folder: replays log to `firmware/sim/flights/` and
read `firmware/sim/pilot.txt` (both ignored by git). `--card DIR` uses another
folder. Every start or loop of a replay is a new flight file. Headless runs
(`--shots`, `--trace`, `--boot-frames`) use a temporary card.
