# Configuration

## Monitors
| Key | Value | Description |
|---|---|---|
| port | `HDMI-1` | the name of the port of the output monitor |
| primary | `true` | set the monitor as primary |
| position | See `position segment` |

### Monitor Positions
| Value | Description |
|---|---|
| `( x: 0, y: 0 )` | exact pixel position |
| `( rightOf: "PORT" )` | relative position to the right of specified port |
| `( leftOf: "PORT" )` | relative position to the left of specified port |
| `( above: "PORT" )` | relative position to the above specified port |
| `( below: "PORT" )` | relative position to the belowe specified port |

### Example
```
(
  monitors: [
		(
            port: "e-DPI",
            primary: true
        ),
		(
			port: "HDMI-1", 
			position: ( rightOf: "eDPI" ),
		),
	],
)
```

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

## Key-Bindings

### Actions

#### `focusWindow`
focus the nearest window given the direction
```
  +---+  +---+
  | 1 |  | 2 |
  +---+  +---+
```
if the currely focused window is number 1, when called `( focus: "right" )` it will focus on number 2.
```
HDMI-1           HDMI-2
+--------------+ +--------------+
| +---+  +---+ | | +---+  +---+ |
| | 1 |  | 2 | | | | 3 |  | 4 | |
| +---+  +---+ | | +---+  +---+ |
+--------------+ +--------------+
```
if the currely focused window is number 2 and you specified `HDMI-2` to be the right of `HDMI-1`,
when called `( focus: "right" )` it will focus on number 3 and automatically focus on the `HDMI-2` monitor.


### Example
```
(
	keyBindings: {

		"alt + return": ( exec: ["xterm"] ),
		"alt + space": ( exec: ["dmenu"] ),

		"alt + left": ( focusWindow: "left" ),
		"alt + right": ( focusWindow: "right" ),
		"alt + up": ( focusWindow: "top" ),
		"alt + down": ( focusWindow: "bottom" ),

		"alt + shift + left": ( tileWindow: [0.0, 0.0, 0.5, 1.0] ),
		"alt + shift + right": ( tileWindow: [0.5, 0.0, 0.5, 1.0] ),
		"alt + shift + up": ( maximizeWindow: null ),
		"alt + shift + down": ( restoreWindow: null ),

		"alt + 1": ( focusWorkspace: "1" ),
		"alt + 2": ( focusWorkspace: "2" ),
		"alt + 3": ( focusWorkspace: "3" ),
		"alt + 4": ( focusWorkspace: "4" ),
		"alt + 5": ( focusWorkspace: "5" ),

		"alt + shift + 1": ( moveWindowToWorkspace: "1" ),
		"alt + shift + 2": ( moveWindowToWorkspace: "2" ),
		"alt + shift + 3": ( moveWindowToWorkspace: "3" ),
		"alt + shift + 4": ( moveWindowToWorkspace: "4" ),
		"alt + shift + 5": ( moveWindowToWorkspace: "5" ),

	},
)
```

## Mouse-Binding

### Example
```
(
	mouseBindings: {
        "click": ( focusWindow: null ),
        "alt + drag": ( moveWindow: null ),
        "alt + shift + drag": ( resizeWindow: null ),
	},
)
```
