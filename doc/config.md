# Configuration

## Workspaces

### Example
```
(
	workspaces: [
		( id: "1" ), # if you are not specify the port, it will use the primary monitor
		( id: "2" ),
		( id: "3" ),
		( id: "4" ),
		( id: "5", port: "HDMI-1" ),
	],
)
```

## Bindings

### Commands

#### `focusWindow`
focus the nearest window given the direction
```
HDMI-1
+--------------+
| +---+  +---+ |
| | 1 |  | 2 | |
| +---+  +---+ |
+--------------+
```
if the currely focused window is number 1, when called `( focus: "east" )` it will focus on number 2.
```
HDMI-1           HDMI-2
+--------------+ +--------------+
| +---+  +---+ | | +---+  +---+ |
| | 1 |  | 2 | | | | 3 |  | 4 | |
| +---+  +---+ | | +---+  +---+ |
+--------------+ +--------------+
```
if the currely focused window is number 2 and you specified `HDMI-2` to be the right of `HDMI-1`,
when called `( focus: "east" )` it will focus on number 3 and automatically focus on the `HDMI-2` monitor.



### Example
```
(
	bindings: {

		"super + return": ( exec: ["xterm"] ),
		"super + space":  ( exec: ["dmenu"] ),

		"super + backspace": ( closeWindow: null ),

		"super + left":  ( focusWindow: "west" ),
		"super + right": ( focusWindow: "east" ),
		"super + up":    ( focusWindow: "north" ),
		"super + down":  ( focusWindow: "south" ),

		"super + shift + left":  ( tileWindow: [0.0, 0.0, 0.5, 1.0] ),
		"super + shift + right": ( tileWindow: [0.5, 0.0, 0.5, 1.0] ),
		"super + shift + up":    ( maximizeWindow: null ),
		"super + shift + down":  ( restoreWindow: null ),

		"super + 1": ( focusWorkspace: "1" ),
		"super + 2": ( focusWorkspace: "2" ),
		"super + 3": ( focusWorkspace: "3" ),
		"super + 4": ( focusWorkspace: "4" ),
		"super + 5": ( focusWorkspace: "5" ),

		"super + shift + 1": ( sendWindowToWorkspace: "1" ),
		"super + shift + 2": ( sendWindowToWorkspace: "2" ),
		"super + shift + 3": ( sendWindowToWorkspace: "3" ),
		"super + shift + 4": ( sendWindowToWorkspace: "4" ),
		"super + shift + 5": ( sendWindowToWorkspace: "5" ),

	},
)
```
