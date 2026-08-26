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
	bindings: [

		( key: "alt + return", exec: ["xterm"] ),
		( key: "alt + space", exec: ["dmenu"] ),

		( key: "alt + backspace", closeWindow: null ),

		( key: "alt + left", focusWindow: "west" ),
		( key: "alt + right", focusWindow: "east" ),
		( key: "alt + up", focusWindow: "north" ),
		( key: "alt + down", focusWindow: "south" ),

		( key: "alt + shift + left", tileWindow: [0.0, 0.0, 0.5, 1.0] ),
		( key: "alt + shift + right", tileWindow: [0.5, 0.0, 0.5, 1.0] ),
		( key: "alt + shift + up", maximizeWindow: null ),
		( key: "alt + shift + down", restoreWindow: null ),

		( key: "alt + 1", focusWorkspace: "1" ),
		( key: "alt + 2", focusWorkspace: "2" ),
		( key: "alt + 3", focusWorkspace: "3" ),
		( key: "alt + 4", focusWorkspace: "4" ),
		( key: "alt + 5", focusWorkspace: "5" ),

		( key: "alt + shift + 1", sendWindowToWorkspace: "1" ),
		( key: "alt + shift + 2", sendWindowToWorkspace: "2" ),
		( key: "alt + shift + 3", sendWindowToWorkspace: "3" ),
		( key: "alt + shift + 4", sendWindowToWorkspace: "4" ),
		( key: "alt + shift + 5", sendWindowToWorkspace: "5" ),

	],
)
```
