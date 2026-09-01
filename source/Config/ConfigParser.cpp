#include "Common/Direction.hpp"
#include "Common/Logger.hpp"
#include "Config/Config.hpp"
#include "Config/ConfigParser.hpp"
#include "Input/Key.hpp"
#include "Wx/Serializer/WkValue.hpp"

Config ParseConfig(Wx::WkValue const& value) {
	struct Parser {
		Config config;

		void parse(Wx::WkValue const& value) {
			if (!value.IsTuple()) {
				return;
			}
			Wx::HashTable<Wx::String, Wx::WkValue> const& tup = value.AsTuple();
			Wx::WkValue* ws = tup.TryGet("workspaces");
			if (ws) {
				parseWorkspaceList(*ws);
			}

			Wx::WkValue* bindings = tup.TryGet("bindings");
			if (bindings) {
				parseBindingList(*bindings);
			}

			Wx::WkValue* autostarts = tup.TryGet("autostarts");
			if (autostarts) {
				parseAutostartList(*autostarts);
			}
		}

		void parseWorkspaceList(Wx::WkValue const& value) {
			if (!value.IsList()) {
				return;
			}
			Wx::List<Wx::WkValue> const& list = value.AsList();
			for (int i = 0; i < list.Count(); i++) {
				parseWorkspace(list[i]);
			}
		}

		void parseWorkspace(Wx::WkValue const& value) {
			if (!value.IsTuple()) {
				return;
			}
			Wx::HashTable<Wx::String, Wx::WkValue> const& tup = value.AsTuple();
			Wx::WkValue* id = tup.TryGet("id");
			Wx::WkValue* port = tup.TryGet("port");

			if (id && id->IsString()) {
				ConfigWorkspace ws;
				ws.id = id->AsString();
				if (port && port->IsString()) {
					ws.ports.Append(port->AsString());
				}
				config.workspaces.Append(ws);
			}
		}

		void parseBindingList(Wx::WkValue const& value) {
			if (!value.IsHashTable()) return;

			Wx::HashTable<Wx::String, Wx::WkValue> const& table = value.AsHashTable();
			for (auto it = table.Iterate(); it.HasNext(); it.Next()) {
				parseBinding(it.Get().key, it.Get().value);
			}
		}

		void parseBinding(Wx::String const& key, Wx::WkValue const& value) {
			if (!value.IsTuple()) return;

			ConfigBinding binding;

			Wx::HashTable<Wx::String, Wx::WkValue> const& tup = value.AsTuple();

			Wx::List<Wx::Span<Wx::Byte>> spans = splitKey(key, '+');
			if (spans.Count() == 1 || spans.Count() == 0) {
				LOG("[config] invalid key combination: %s\n", key.CPtr());
				return;
			}
			ModKey modKey = ModKey::None;
			for (int i = 0; i < spans.Count() - 1; i++) {
				ModKey mk = stringToModKey(Wx::String(spans[i].Count(), spans[i].Ptr()));
				if (mk == ModKey::None) {
					LOG("[config] invalid mod-key: %s\n", key.CPtr());
					return;
				}
				modKey |= mk;
			}
			Key k = stringToKey(Wx::String(spans.GetLast().Count(), spans.GetLast().Ptr()));
			if (k == Key::None) {
				LOG("[config] invalid key: %s\n", key.CPtr());
				return;
			}

			binding.modKey = modKey;
			binding.key = k;

			ConfigCommand command = parseCommand(tup);
			if (command.func != ConfigCommandFunc::None) {
				binding.commands.Append(command);
			}

			if (binding.commands.IsEmpty()) {
				return;
			}

			config.bindings.Append(binding);
		}

		ConfigCommand parseCommand(Wx::HashTable<Wx::String, Wx::WkValue> const& tup) {
			{
				Wx::WkValue* exec = tup.TryGet("exec");
				if (exec) {
					if (!exec->IsList()) {
						LOG("[config] invalid exec command argument, expect list\n");
						return {};
					}
					for (int i = 0; i < exec->AsList().Count(); i++) {
						if (!exec->AsList()[i].IsString()) {
							LOG("[config] invalid exec command argument, expect list of string\n");
							return {};
						}
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::Exec;
					command.arguments = *exec;

					return command;
				}
			}

			{
				Wx::WkValue* closeWindow = tup.TryGet("closeWindow");
				if (closeWindow) {
					if (!closeWindow->IsNull()) {
						LOG("[config] invalid closeWindow command argument, expect null\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::CloseWindow;
					return command;
				}
			}

			{
				Wx::WkValue* cycleNextWindow = tup.TryGet("cycleNextWindow");
				if (cycleNextWindow) {
					if (!cycleNextWindow->IsNull()) {
						LOG("[config] invalid cycleNextWindow command argument, expect null\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::CycleNextWindow;
					return command;
				}
			}

			{
				Wx::WkValue* cyclePreviousWindow = tup.TryGet("cyclePreviousWindow");
				if (cyclePreviousWindow) {
					if (!cyclePreviousWindow->IsNull()) {
						LOG("[config] invalid cyclePreviousWindow command argument, expect null\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::CyclePreviousWindow;
					return command;
				}
			}

			{
				Wx::WkValue* focusWindow = tup.TryGet("focusWindow");
				if (focusWindow) {
					if (!focusWindow->IsString()) {
						LOG("[config] invalid focusWindow command argument, expect string\n");
						return {};
					}

					Wx::WkValue direction;

					if (focusWindow->AsString() == "west") {
						direction = Wx::WkValue::CreateInt64((int) Direction::West);
					} else if (focusWindow->AsString() == "east") {
						direction = Wx::WkValue::CreateInt64((int) Direction::East);
					} else if (focusWindow->AsString() == "north") {
						direction = Wx::WkValue::CreateInt64((int) Direction::North);
					} else if (focusWindow->AsString() == "south") {
						direction = Wx::WkValue::CreateInt64((int) Direction::South);
					} else {
						LOG("[config] invalid focusWindow command argument, expect 'west', 'east', 'north', 'south' \n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::FocusWindow;
					command.arguments = direction;
					return command;
				}
			}

			{
				Wx::WkValue* tileWindow = tup.TryGet("tileWindow");
				if (tileWindow) {
					if (!tileWindow->IsList()) {
						LOG("[config] invalid tileWindow command argument, expect list");
						return {};
					}
					if (tileWindow->AsList().Count() != 4) {
						LOG("[config] invalid tileWindow command argument, expect list with 4 items");
						return {};
					}
					for (int i = 0; i < tileWindow->AsList().Count(); i++) {
						if (!tileWindow->AsList()[i].IsFloat64()) {
							LOG("[config] invalid tileWindow command argument, expect list of float\n");
							return {};
						}
					}
					ConfigCommand command;
					command.func = ConfigCommandFunc::TileWindow;
					command.arguments = *tileWindow;

					return command;
				}
			}

			{
				Wx::WkValue* focusWorkspace = tup.TryGet("focusWorkspace");
				if (focusWorkspace) {
					if (!focusWorkspace->IsString()) {
						LOG("[config] invalid focusWorkspace command argument, expect string\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::FocusWorkspace;
					command.arguments = *focusWorkspace;

					return command;
				}
			}

			{
				Wx::WkValue* sendWindowToWorkspace = tup.TryGet("sendWindowToWorkspace");
				if (sendWindowToWorkspace) {
					if (!sendWindowToWorkspace->IsString()) {
						LOG("[config] invalid sendWindowToWorkspace command argument, expect string\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::SendWindowToWorkspace;
					command.arguments = *sendWindowToWorkspace;

					return command;
				}
			}

			{
				Wx::WkValue* quit = tup.TryGet("quit");
				if (quit) {
					if (!quit->IsNull()) {
						LOG("[config] invalid quit command argument, expect null\n");
						return {};
					}

					ConfigCommand command;
					command.func = ConfigCommandFunc::Quit;

					return command;
				}
			}

			return {};
		}

		void parseKey(ConfigBinding& binding, Wx::WkValue const& value) {
			if (!value.IsString()) return;
			Wx::List<Wx::Span<Wx::Byte>> splited = splitKey(value.AsString(), '+');
		}

		Wx::Span<Wx::Byte> trimSpace(Wx::Span<Wx::Byte> span) {
			int k = 0;
			for (; k < span.Count(); k++) {
				if (span[k] != ' ') {
					break;
				}
			}
			int j = span.Count() - 1;
			for (; j > k; j--) {
				if (span[j] != ' ') {
					break;
				}
			}
			return span.slice(k, j - k + 1);
		}

		Wx::List<Wx::Span<Wx::Byte>> splitKey(Wx::String const& str, Wx::Byte needle) {
			Wx::List<Wx::Span<Wx::Byte>> list;
			int i = 0;
			int run = 0;
			Wx::Span<Wx::Byte> span = str.ToSpan();
			while (i < span.Count()) {
				if (span[i] == needle) {
					list.Append(trimSpace(span.slice(run, i - run)));
					run = i + 1;
				}
				i++;
			}
			if (run < span.Count()) {
				list.Append(trimSpace(span.slice(run, span.Count() - run)));
			}
			return list;
		}

		Key stringToKey(Wx::String const& key) {
			if (key == "backspace") return Key::Backspace;
			if (key == "tab") return Key::Tab;
			if (key == "linefeed") return Key::Linefeed;
			if (key == "clear") return Key::Clear;
			if (key == "return") return Key::Return;
			if (key == "pause") return Key::Pause;
			if (key == "scrollLock") return Key::ScrollLock;
			if (key == "sysReq") return Key::SysReq;
			if (key == "escape") return Key::Escape;
			if (key == "delete") return Key::Delete;

			if (key == "multiKeyCompose") return Key::MultiKeyCompose;
			if (key == "codeinput") return Key::Codeinput;
			if (key == "singleCandidate") return Key::SingleCandidate;
			if (key == "multipleCandidate") return Key::MultipleCandidate;
			if (key == "previousCandidate") return Key::PreviousCandidate;

			if (key == "home") return Key::Home;
			if (key == "left") return Key::Left;
			if (key == "up") return Key::Up;
			if (key == "right") return Key::Right;
			if (key == "down") return Key::Down;
			if (key == "prior") return Key::Prior;
			if (key == "pageUp") return Key::PageUp;
			if (key == "next") return Key::Next;
			if (key == "pageDown") return Key::PageDown;
			if (key == "end") return Key::End;
			if (key == "begin") return Key::Begin;

			if (key == "select") return Key::Select;
			if (key == "print") return Key::Print;
			if (key == "execute") return Key::Execute;
			if (key == "insert") return Key::Insert;
			if (key == "undo") return Key::Undo;
			if (key == "redo") return Key::Redo;
			if (key == "menu") return Key::Menu;
			if (key == "find") return Key::Find;
			if (key == "cancel") return Key::Cancel;
			if (key == "help") return Key::Help;
			if (key == "break") return Key::Break;
			if (key == "modeSwitch") return Key::ModeSwitch;
			if (key == "scriptSwitch") return Key::ScriptSwitch;
			if (key == "numLock") return Key::NumLock;

			if (key == "kp.space") return Key::KP_Space;
			if (key == "kp.tab") return Key::KP_Tab;
			if (key == "kp.enter") return Key::KP_Enter;
			if (key == "kp.f1") return Key::KP_F1;
			if (key == "kp.f2") return Key::KP_F2;
			if (key == "kp.f3") return Key::KP_F3;
			if (key == "kp.f4") return Key::KP_F4;
			if (key == "kp.home") return Key::KP_Home;
			if (key == "kp.left") return Key::KP_Left;
			if (key == "kp.up") return Key::KP_Up;
			if (key == "kp.right") return Key::KP_Right;
			if (key == "kp.down") return Key::KP_Down;
			if (key == "kp.prior") return Key::KP_Prior;
			if (key == "kp.pageUp") return Key::KP_PageUp;
			if (key == "kp.Next") return Key::KP_Next;
			if (key == "kp.pageDown") return Key::KP_PageDown;
			if (key == "kp.end") return Key::KP_End;
			if (key == "kp.begin") return Key::KP_Begin;
			if (key == "kp.Insert") return Key::KP_Insert;
			if (key == "kp.delete") return Key::KP_Delete;
			if (key == "kp.equal") return Key::KP_Equal;
			if (key == "kp.multiply") return Key::KP_Multiply;
			if (key == "kp.add") return Key::KP_Add;
			if (key == "kp.separator") return Key::KP_Separator;
			if (key == "kp.subtract") return Key::KP_Subtract;
			if (key == "kp.decimal") return Key::KP_Decimal;
			if (key == "kp.divide") return Key::KP_Divide;

			if (key == "kp.0") return Key::KP_0;
			if (key == "kp.1") return Key::KP_1;
			if (key == "kp.2") return Key::KP_2;
			if (key == "kp.3") return Key::KP_3;
			if (key == "kp.4") return Key::KP_4;
			if (key == "kp.5") return Key::KP_5;
			if (key == "kp.6") return Key::KP_6;
			if (key == "kp.7") return Key::KP_7;
			if (key == "kp.8") return Key::KP_8;
			if (key == "kp.9") return Key::KP_9;

			if (key == "f1") return Key::F1;
			if (key == "f2") return Key::F2;
			if (key == "f3") return Key::F3;
			if (key == "f4") return Key::F4;
			if (key == "f5") return Key::F5;
			if (key == "f6") return Key::F6;
			if (key == "f7") return Key::F7;
			if (key == "f8") return Key::F8;
			if (key == "f9") return Key::F9;
			if (key == "f10") return Key::F10;
			if (key == "f11") return Key::F11;
			if (key == "f12") return Key::F12;
			if (key == "f13") return Key::F13;
			if (key == "f14") return Key::F14;
			if (key == "f15") return Key::F15;
			if (key == "f16") return Key::F16;
			if (key == "f17") return Key::F17;
			if (key == "f18") return Key::F18;
			if (key == "f19") return Key::F19;
			if (key == "f20") return Key::F20;
			if (key == "f21") return Key::F21;
			if (key == "f22") return Key::F22;
			if (key == "f23") return Key::F23;
			if (key == "f24") return Key::F24;
			if (key == "f25") return Key::F25;
			if (key == "f26") return Key::F26;
			if (key == "f27") return Key::F27;
			if (key == "f28") return Key::F28;
			if (key == "f29") return Key::F29;
			if (key == "f30") return Key::F30;
			if (key == "f31") return Key::F31;
			if (key == "f32") return Key::F32;
			if (key == "f33") return Key::F33;
			if (key == "f34") return Key::F34;
			if (key == "f35") return Key::F35;

			if (key == "shiftL") return Key::ShiftL;
			if (key == "shiftR") return Key::ShiftR;
			if (key == "ctrlL") return Key::CtrlL;
			if (key == "ctrlR") return Key::CtrlR;
			if (key == "capsLock") return Key::CapsLock;
			if (key == "shiftLock") return Key::ShiftLock;

			if (key == "metaL") return Key::MetaL;
			if (key == "metaR") return Key::MetaR;
			if (key == "altL") return Key::AltL;
			if (key == "altR") return Key::AltR;
			if (key == "superL") return Key::SuperL;
			if (key == "superR") return Key::SuperR;
			if (key == "hyperL") return Key::HyperL;
			if (key == "hyperR") return Key::HyperR;

			if (key == "iso.lock") return Key::ISO_Lock;
			if (key == "iso.level2Latch") return Key::ISO_Level2Latch;
			if (key == "iso.level3Shift") return Key::ISO_Level3Shift;
			if (key == "iso.level3Latch") return Key::ISO_Level3Latch;
			if (key == "iso.level3Lock") return Key::ISO_Level3Lock;
			if (key == "iso.level5Shift") return Key::ISO_Level5Shift;
			if (key == "iso.level5Latch") return Key::ISO_Level5Latch;
			if (key == "iso.level5Lock") return Key::ISO_Level5Lock;
			if (key == "iso.groupShift") return Key::ISO_GroupShift;
			if (key == "iso.groupLatch") return Key::ISO_GroupLatch;
			if (key == "iso.groupLock") return Key::ISO_GroupLock;
			if (key == "iso.NextGroup") return Key::ISO_NextGroup;
			if (key == "iso.NextGroupLock") return Key::ISO_NextGroupLock;
			if (key == "iso.prevGroup") return Key::ISO_PrevGroup;
			if (key == "iso.prevGroupLock") return Key::ISO_PrevGroupLock;
			if (key == "iso.firstGroup") return Key::ISO_FirstGroup;
			if (key == "iso.firstGroupLock") return Key::ISO_FirstGroupLock;
			if (key == "iso.lastGroup") return Key::ISO_LastGroup;
			if (key == "iso.lastGroupLock") return Key::ISO_LastGroupLock;

			if (key == "iso.leftTab") return Key::ISO_LeftTab;
			if (key == "iso.moveLineUp") return Key::ISO_MoveLineUp;
			if (key == "iso.moveLineDown") return Key::ISO_MoveLineDown;
			if (key == "iso.partialLineUp") return Key::ISO_PartialLineUp;
			if (key == "iso.partialLineDown") return Key::ISO_PartialLineDown;
			if (key == "iso.partialSpaceLeft") return Key::ISO_PartialSpaceLeft;
			if (key == "iso.partialSpaceRight") return Key::ISO_PartialSpaceRight;
			if (key == "iso.SetMarginLeft") return Key::ISO_SetMarginLeft;
			if (key == "iso.SetMarginRight") return Key::ISO_SetMarginRight;
			if (key == "iso.releaseMarginLeft") return Key::ISO_ReleaseMarginLeft;
			if (key == "iso.releaseMarginRight") return Key::ISO_ReleaseMarginRight;
			if (key == "iso.releaseBothMargins") return Key::ISO_ReleaseBothMargins;
			if (key == "iso.fastCursorLeft") return Key::ISO_FastCursorLeft;
			if (key == "iso.fastCursorRight") return Key::ISO_FastCursorRight;
			if (key == "iso.fastCursorUp") return Key::ISO_FastCursorUp;
			if (key == "iso.fastCursorDown") return Key::ISO_FastCursorDown;
			if (key == "iso.continuousUnderline") return Key::ISO_ContinuousUnderline;
			if (key == "iso.discontinuousUnderline") return Key::ISO_DiscontinuousUnderline;
			if (key == "iso.emphasize") return Key::ISO_Emphasize;
			if (key == "iso.centerObject") return Key::ISO_CenterObject;
			if (key == "iso.enter") return Key::ISO_Enter;

			if (key == "deadGrave") return Key::DeadGrave;
			if (key == "deadAcute") return Key::DeadAcute;
			if (key == "deadCircumflex") return Key::DeadCircumflex;
			if (key == "deadTilde") return Key::DeadTilde;
			if (key == "deadPerispomeni") return Key::DeadPerispomeni;
			if (key == "deadMacron") return Key::DeadMacron;
			if (key == "deadBreve") return Key::DeadBreve;
			if (key == "deadAboveDot") return Key::DeadAboveDot;
			if (key == "deadDiaeresis") return Key::DeadDiaeresis;
			if (key == "deadAboveRing") return Key::DeadAboveRing;
			if (key == "deadDoubleAcute") return Key::DeadDoubleAcute;
			if (key == "deadCaron") return Key::DeadCaron;
			if (key == "deadCedilla") return Key::DeadCedilla;
			if (key == "deadOgonek") return Key::DeadOgonek;
			if (key == "deadIota") return Key::DeadIota;
			if (key == "deadVoicedSound") return Key::DeadVoicedSound;
			if (key == "deadSemivoicedSound") return Key::DeadSemivoicedSound;
			if (key == "deadBelowDot") return Key::DeadBelowDot;
			if (key == "deadHook") return Key::DeadHook;
			if (key == "deadHorn") return Key::DeadHorn;
			if (key == "deadStroke") return Key::DeadStroke;
			if (key == "deadAboveComma") return Key::DeadAboveComma;
			if (key == "deadPsili") return Key::DeadPsili;
			if (key == "deadAboveReversedComma") return Key::DeadAboveReversedComma;
			if (key == "deadDasia") return Key::DeadDasia;
			if (key == "deadDoubleGrave") return Key::DeadDoubleGrave;
			if (key == "deadBelowRing") return Key::DeadBelowRing;
			if (key == "deadBelowMacron") return Key::DeadBelowMacron;
			if (key == "deadBelowCircumflex") return Key::DeadBelowCircumflex;
			if (key == "deadBelowTilde") return Key::DeadBelowTilde;
			if (key == "deadBelowBreve") return Key::DeadBelowBreve;
			if (key == "deadBelowDiaeresis") return Key::DeadBelowDiaeresis;
			if (key == "deadInvertedBreve") return Key::DeadInvertedBreve;
			if (key == "deadBelowComma") return Key::DeadBelowComma;
			if (key == "deadCurrency") return Key::DeadCurrency;

			if (key == "deadLowLine") return Key::DeadLowLine;
			if (key == "deadAboveVerticalLine") return Key::DeadAboveVerticalLine;
			if (key == "deadBelowVerticalLine") return Key::DeadBelowVerticalLine;
			if (key == "deadLongSolidusOverlay") return Key::DeadLongSolidusOverlay;

			if (key == "deadSmallA") return Key::DeadSmallA;
			if (key == "deadA") return Key::DeadA;
			if (key == "deadSmallE") return Key::DeadSmallE;
			if (key == "deadE") return Key::DeadE;
			if (key == "deadSmallI") return Key::DeadSmallI;
			if (key == "deadI") return Key::DeadI;
			if (key == "deadSmallO") return Key::DeadSmallO;
			if (key == "deadO") return Key::DeadO;
			if (key == "deadSmallU") return Key::DeadSmallU;
			if (key == "deadU") return Key::DeadU;
			if (key == "deadSmallSchwa") return Key::DeadSmallSchwa;
			if (key == "deadSchwa") return Key::DeadSchwa;

			if (key == "deadGreek") return Key::DeadGreek;
			if (key == "deadHamza") return Key::DeadHamza;
			if (key == "deadApostrophe") return Key::DeadApostrophe;

			if (key == "firstVirtualScreen") return Key::FirstVirtualScreen;
			if (key == "prevVirtualScreen") return Key::PrevVirtualScreen;
			if (key == "nextVirtualScreen") return Key::NextVirtualScreen;
			if (key == "lastVirtualScreen") return Key::LastVirtualScreen;
			if (key == "terminateServer") return Key::TerminateServer;

			if (key == "accessXEnable") return Key::AccessXEnable;
			if (key == "accessXFeedbackEnable") return Key::AccessXFeedbackEnable;
			if (key == "repeatKeysEnable") return Key::RepeatKeysEnable;
			if (key == "slowKeysEnable") return Key::SlowKeysEnable;
			if (key == "bounceKeysEnable") return Key::BounceKeysEnable;
			if (key == "stickyKeysEnable") return Key::StickyKeysEnable;
			if (key == "mouseKeysEnable") return Key::MouseKeysEnable;
			if (key == "mouseKeysAccelEnable") return Key::MouseKeysAccelEnable;
			if (key == "overlay1Enable") return Key::Overlay1Enable;
			if (key == "overlay2Enable") return Key::Overlay2Enable;
			if (key == "audibleBellEnable") return Key::AudibleBellEnable;

			if (key == "pointer.left") return Key::Pointer_Left;
			if (key == "pointer.right") return Key::Pointer_Right;
			if (key == "pointer.up") return Key::Pointer_Up;
			if (key == "pointer.down") return Key::Pointer_Down;
			if (key == "pointer.upLeft") return Key::Pointer_UpLeft;
			if (key == "pointer.upRight") return Key::Pointer_UpRight;
			if (key == "pointer.downLeft") return Key::Pointer_DownLeft;
			if (key == "pointer.downRight") return Key::Pointer_DownRight;
			if (key == "pointer.button_Dflt") return Key::Pointer_Button_Dflt;
			if (key == "pointer.button1") return Key::Pointer_Button1;
			if (key == "pointer.button2") return Key::Pointer_Button2;
			if (key == "pointer.button3") return Key::Pointer_Button3;
			if (key == "pointer.button4") return Key::Pointer_Button4;
			if (key == "pointer.button5") return Key::Pointer_Button5;
			if (key == "pointer.dblClick_Dflt") return Key::Pointer_DblClick_Dflt;
			if (key == "pointer.dblClick1") return Key::Pointer_DblClick1;
			if (key == "pointer.dblClick2") return Key::Pointer_DblClick2;
			if (key == "pointer.dblClick3") return Key::Pointer_DblClick3;
			if (key == "pointer.dblClick4") return Key::Pointer_DblClick4;
			if (key == "pointer.dblClick5") return Key::Pointer_DblClick5;
			if (key == "pointer.drag_Dflt") return Key::Pointer_Drag_Dflt;
			if (key == "pointer.drag1") return Key::Pointer_Drag1;
			if (key == "pointer.drag2") return Key::Pointer_Drag2;
			if (key == "pointer.drag3") return Key::Pointer_Drag3;
			if (key == "pointer.drag4") return Key::Pointer_Drag4;
			if (key == "pointer.drag5") return Key::Pointer_Drag5;

			if (key == "pointer.enableKeys") return Key::Pointer_EnableKeys;
			if (key == "pointer.accelerate") return Key::Pointer_Accelerate;
			if (key == "pointer.dfltBtnNext") return Key::Pointer_DfltBtnNext;
			if (key == "pointer.dfltBtnPrev") return Key::Pointer_DfltBtnPrev;

			if (key == "digraphSmallSmall") return Key::DigraphSmallSmall;
			if (key == "digraphCapSmall") return Key::DigraphCapSmall;
			if (key == "digraphCapCap") return Key::DigraphCapCap;
			if (key == "nGraphSmallSmall") return Key::NGraphSmallSmall;
			if (key == "nGraphCapSmall") return Key::NGraphCapSmall;
			if (key == "nGraphCapCap") return Key::NGraphCapCap;

			if (key == "t3270.duplicate") return Key::T3270_Duplicate;
			if (key == "t3270.fieldMark") return Key::T3270_FieldMark;
			if (key == "t3270.right2") return Key::T3270_Right2;
			if (key == "t3270.left2") return Key::T3270_Left2;
			if (key == "t3270.backTab") return Key::T3270_BackTab;
			if (key == "t3270.eraseEOF") return Key::T3270_EraseEOF;
			if (key == "t3270.eraseInput") return Key::T3270_EraseInput;
			if (key == "t3270.reset") return Key::T3270_Reset;
			if (key == "t3270.quit") return Key::T3270_Quit;
			if (key == "t3270.pA1") return Key::T3270_PA1;
			if (key == "t3270.pA2") return Key::T3270_PA2;
			if (key == "t3270.pA3") return Key::T3270_PA3;
			if (key == "t3270.test") return Key::T3270_Test;
			if (key == "t3270.attn") return Key::T3270_Attn;
			if (key == "t3270.cursorBlink") return Key::T3270_CursorBlink;
			if (key == "t3270.altCursor") return Key::T3270_AltCursor;
			if (key == "t3270.keyClick") return Key::T3270_KeyClick;
			if (key == "t3270.jump") return Key::T3270_Jump;
			if (key == "t3270.ident") return Key::T3270_Ident;
			if (key == "t3270.rule") return Key::T3270_Rule;
			if (key == "t3270.copy") return Key::T3270_Copy;
			if (key == "t3270.play") return Key::T3270_Play;
			if (key == "t3270.Setup") return Key::T3270_Setup;
			if (key == "t3270.record") return Key::T3270_Record;
			if (key == "t3270.changeScreen") return Key::T3270_ChangeScreen;
			if (key == "t3270.deleteWord") return Key::T3270_DeleteWord;
			if (key == "t3270.exSelect") return Key::T3270_ExSelect;
			if (key == "t3270.cursorSelect") return Key::T3270_CursorSelect;
			if (key == "t3270.printScreen") return Key::T3270_PrintScreen;
			if (key == "t3270.enter") return Key::T3270_Enter;

			if (key == "space") return Key::Space;
			if (key == "exclam") return Key::Exclam;
			if (key == "doubleQuote") return Key::DoubleQuote;
			if (key == "numbersign") return Key::Numbersign;
			if (key == "dollar") return Key::Dollar;
			if (key == "percent") return Key::Percent;
			if (key == "ampersand") return Key::Ampersand;
			if (key == "apostrophe") return Key::Apostrophe;
			if (key == "parenL") return Key::ParenL;
			if (key == "parenR") return Key::ParenR;
			if (key == "asterisk") return Key::Asterisk;
			if (key == "plus") return Key::Plus;
			if (key == "comma") return Key::Comma;
			if (key == "minus") return Key::Minus;
			if (key == "period") return Key::Period;
			if (key == "slash") return Key::Slash;
			if (key == "0") return Key::Num0;
			if (key == "1") return Key::Num1;
			if (key == "2") return Key::Num2;
			if (key == "3") return Key::Num3;
			if (key == "4") return Key::Num4;
			if (key == "5") return Key::Num5;
			if (key == "6") return Key::Num6;
			if (key == "7") return Key::Num7;
			if (key == "8") return Key::Num8;
			if (key == "9") return Key::Num9;
			if (key == "colon") return Key::Colon;
			if (key == "semicolon") return Key::Semicolon;
			if (key == "less") return Key::Less;
			if (key == "equal") return Key::Equal;
			if (key == "greater") return Key::Greater;
			if (key == "question") return Key::Question;
			if (key == "at") return Key::At;
			if (key == "a") return Key::A;
			if (key == "b") return Key::B;
			if (key == "c") return Key::C;
			if (key == "d") return Key::D;
			if (key == "e") return Key::E;
			if (key == "f") return Key::F;
			if (key == "g") return Key::G;
			if (key == "h") return Key::H;
			if (key == "i") return Key::I;
			if (key == "j") return Key::J;
			if (key == "k") return Key::K;
			if (key == "l") return Key::L;
			if (key == "m") return Key::M;
			if (key == "n") return Key::N;
			if (key == "o") return Key::O;
			if (key == "p") return Key::P;
			if (key == "q") return Key::Q;
			if (key == "r") return Key::R;
			if (key == "s") return Key::S;
			if (key == "t") return Key::T;
			if (key == "u") return Key::U;
			if (key == "v") return Key::V;
			if (key == "w") return Key::W;
			if (key == "x") return Key::X;
			if (key == "y") return Key::Y;
			if (key == "z") return Key::Z;
			if (key == "bracketL") return Key::BracketL;
			if (key == "backslash") return Key::Backslash;
			if (key == "bracketR") return Key::BracketR;
			if (key == "circumflex") return Key::Circumflex;
			if (key == "underscore") return Key::Underscore;
			if (key == "grave") return Key::Grave;
			if (key == "smallA") return Key::SmallA;
			if (key == "smallB") return Key::SmallB;
			if (key == "smallC") return Key::SmallC;
			if (key == "smallD") return Key::SmallD;
			if (key == "smallE") return Key::SmallE;
			if (key == "smallF") return Key::SmallF;
			if (key == "smallG") return Key::SmallG;
			if (key == "smallH") return Key::SmallH;
			if (key == "smallI") return Key::SmallI;
			if (key == "smallJ") return Key::SmallJ;
			if (key == "smallK") return Key::SmallK;
			if (key == "smallL") return Key::SmallL;
			if (key == "smallM") return Key::SmallM;
			if (key == "smallN") return Key::SmallN;
			if (key == "smallO") return Key::SmallO;
			if (key == "smallP") return Key::SmallP;
			if (key == "smallQ") return Key::SmallQ;
			if (key == "smallR") return Key::SmallR;
			if (key == "smallS") return Key::SmallS;
			if (key == "smallT") return Key::SmallT;
			if (key == "smallU") return Key::SmallU;
			if (key == "smallV") return Key::SmallV;
			if (key == "smallW") return Key::SmallW;
			if (key == "smallX") return Key::SmallX;
			if (key == "smallY") return Key::SmallY;
			if (key == "smallZ") return Key::SmallZ;
			if (key == "curlyL") return Key::CurlyL;
			if (key == "bar") return Key::Bar;
			if (key == "pipe") return Key::Pipe;
			if (key == "curlyR") return Key::CurlyR;
			if (key == "tilde") return Key::Tilde;

			if (key == "noBreakSpace") return Key::NoBreakSpace;
			if (key == "exclamDown") return Key::ExclamDown;
			if (key == "cent") return Key::Cent;
			if (key == "sterling") return Key::Sterling;
			if (key == "currency") return Key::Currency;
			if (key == "yen") return Key::Yen;
			if (key == "brokenBar") return Key::BrokenBar;
			if (key == "section") return Key::Section;
			if (key == "diaeresis") return Key::Diaeresis;
			if (key == "copyright") return Key::Copyright;
			if (key == "ordFeminine") return Key::OrdFeminine;
			if (key == "guillemetL") return Key::GuillemetL;
			if (key == "notSign") return Key::NotSign;
			if (key == "hyphen") return Key::Hyphen;
			if (key == "registered") return Key::Registered;
			if (key == "macron") return Key::Macron;
			if (key == "degree") return Key::Degree;
			if (key == "plusMinus") return Key::PlusMinus;
			if (key == "twoSuperior") return Key::TwoSuperior;
			if (key == "threeSuperior") return Key::ThreeSuperior;
			if (key == "acute") return Key::Acute;
			if (key == "mu") return Key::Mu;
			if (key == "paragraph") return Key::Paragraph;
			if (key == "periodCentered") return Key::PeriodCentered;
			if (key == "cedilla") return Key::Cedilla;
			if (key == "oneSuperior") return Key::OneSuperior;
			if (key == "ordMasculine") return Key::OrdMasculine;
			if (key == "guillemetR") return Key::GuillemetR;
			if (key == "oneQuarter") return Key::OneQuarter;
			if (key == "oneHalf") return Key::OneHalf;
			if (key == "threeQuarters") return Key::ThreeQuarters;
			if (key == "questionDown") return Key::QuestionDown;
			if (key == "agrave") return Key::Agrave;
			if (key == "aacute") return Key::Aacute;
			if (key == "acircumflex") return Key::Acircumflex;
			if (key == "atilde") return Key::Atilde;
			if (key == "adiaeresis") return Key::Adiaeresis;
			if (key == "aring") return Key::Aring;
			if (key == "ae") return Key::Ae;
			if (key == "ccedilla") return Key::Ccedilla;
			if (key == "egrave") return Key::Egrave;
			if (key == "eacute") return Key::Eacute;
			if (key == "ecircumflex") return Key::Ecircumflex;
			if (key == "ediaeresis") return Key::Ediaeresis;
			if (key == "igrave") return Key::Igrave;
			if (key == "iacute") return Key::Iacute;
			if (key == "icircumflex") return Key::Icircumflex;
			if (key == "idiaeresis") return Key::Idiaeresis;
			if (key == "eth") return Key::Eth;
			if (key == "ntilde") return Key::Ntilde;
			if (key == "ograve") return Key::Ograve;
			if (key == "oacute") return Key::Oacute;
			if (key == "ocircumflex") return Key::Ocircumflex;
			if (key == "otilde") return Key::Otilde;
			if (key == "odiaeresis") return Key::Odiaeresis;
			if (key == "multiply") return Key::multiply;
			if (key == "oslash") return Key::Oslash;
			if (key == "ugrave") return Key::Ugrave;
			if (key == "uacute") return Key::Uacute;
			if (key == "ucircumflex") return Key::Ucircumflex;
			if (key == "udiaeresis") return Key::Udiaeresis;
			if (key == "yacute") return Key::Yacute;
			if (key == "thorn") return Key::Thorn;
			if (key == "smallSsharp") return Key::SmallSsharp;
			if (key == "smallAgrave") return Key::SmallAgrave;
			if (key == "smallAacute") return Key::SmallAacute;
			if (key == "smallAcircumflex") return Key::SmallAcircumflex;
			if (key == "smallAtilde") return Key::SmallAtilde;
			if (key == "smallAdiaeresis") return Key::SmallAdiaeresis;
			if (key == "smallAring") return Key::SmallAring;
			if (key == "smallAe") return Key::SmallAe;
			if (key == "smallCcedilla") return Key::SmallCcedilla;
			if (key == "smallEgrave") return Key::SmallEgrave;
			if (key == "smallEacute") return Key::SmallEacute;
			if (key == "smallEcircumflex") return Key::SmallEcircumflex;
			if (key == "smallEdiaeresis") return Key::SmallEdiaeresis;
			if (key == "smallIgrave") return Key::SmallIgrave;
			if (key == "smallIacute") return Key::SmallIacute;
			if (key == "smallIcircumflex") return Key::SmallIcircumflex;
			if (key == "smallIdiaeresis") return Key::SmallIdiaeresis;
			if (key == "smallEth") return Key::SmallEth;
			if (key == "smallNtilde") return Key::SmallNtilde;
			if (key == "smallOgrave") return Key::SmallOgrave;
			if (key == "smallOacute") return Key::SmallOacute;
			if (key == "smallOcircumflex") return Key::SmallOcircumflex;
			if (key == "smallOtilde") return Key::SmallOtilde;
			if (key == "smallOdiaeresis") return Key::SmallOdiaeresis;
			if (key == "smallDivision") return Key::SmallDivision;
			if (key == "smallOslash") return Key::SmallOslash;
			if (key == "smallUgrave") return Key::SmallUgrave;
			if (key == "smallUacute") return Key::SmallUacute;
			if (key == "smallUcircumflex") return Key::SmallUcircumflex;
			if (key == "smallUdiaeresis") return Key::SmallUdiaeresis;
			if (key == "smallYacute") return Key::SmallYacute;
			if (key == "smallThorn") return Key::SmallThorn;
			if (key == "smallYdiaeresis") return Key::SmallYdiaeresis;

			if (key == "xf86.modeLock") return Key::XF86_ModeLock;

			if (key == "xf86.monBrightnessUp") return Key::XF86_MonBrightnessUp;
			if (key == "xf86.monBrightnessDown") return Key::XF86_MonBrightnessDown;
			if (key == "xf86.kbdLightOnOff") return Key::XF86_KbdLightOnOff;
			if (key == "xf86.kbdBrightnessUp") return Key::XF86_KbdBrightnessUp;
			if (key == "xf86.kbdBrightnessDown") return Key::XF86_KbdBrightnessDown;
			if (key == "xf86.monBrightnessCycle") return Key::XF86_MonBrightnessCycle;

			if (key == "xf86.standby") return Key::XF86_Standby;
			if (key == "xf86.audioLowerVolume") return Key::XF86_AudioLowerVolume;
			if (key == "xf86.audioMute") return Key::XF86_AudioMute;
			if (key == "xf86.audioRaiseVolume") return Key::XF86_AudioRaiseVolume;
			if (key == "xf86.audioPlay") return Key::XF86_AudioPlay;
			if (key == "xf86.audioStop") return Key::XF86_AudioStop;
			if (key == "xf86.audioPrev") return Key::XF86_AudioPrev;
			if (key == "xf86.audioNext") return Key::XF86_AudioNext;
			if (key == "xf86.homePage") return Key::XF86_HomePage;
			if (key == "xf86.mail") return Key::XF86_Mail;
			if (key == "xf86.start") return Key::XF86_Start;
			if (key == "xf86.search") return Key::XF86_Search;
			if (key == "xf86.audioRecord") return Key::XF86_AudioRecord;

			if (key == "xf86.calculator") return Key::XF86_Calculator;
			if (key == "xf86.memo") return Key::XF86_Memo;
			if (key == "xf86.toDoList") return Key::XF86_ToDoList;
			if (key == "xf86.calendar") return Key::XF86_Calendar;
			if (key == "xf86.powerDown") return Key::XF86_PowerDown;
			if (key == "xf86.contrastAdjust") return Key::XF86_ContrastAdjust;
			if (key == "xf86.rockerUp") return Key::XF86_RockerUp;
			if (key == "xf86.rockerDown") return Key::XF86_RockerDown;
			if (key == "xf86.rockerEnter") return Key::XF86_RockerEnter;

			if (key == "xf86.back") return Key::XF86_Back;
			if (key == "xf86.forward") return Key::XF86_Forward;
			if (key == "xf86.stop") return Key::XF86_Stop;
			if (key == "xf86.refresh") return Key::XF86_Refresh;
			if (key == "xf86.powerOff") return Key::XF86_PowerOff;
			if (key == "xf86.wakeUp") return Key::XF86_WakeUp;
			if (key == "xf86.eject") return Key::XF86_Eject;
			if (key == "xf86.screenSaver") return Key::XF86_ScreenSaver;
			if (key == "xf86.www") return Key::XF86_WWW;
			if (key == "xf86.sleep") return Key::XF86_Sleep;
			if (key == "xf86.favorites") return Key::XF86_Favorites;
			if (key == "xf86.audioPause") return Key::XF86_AudioPause;
			if (key == "xf86.audioMedia") return Key::XF86_AudioMedia;
			if (key == "xf86.myComputer") return Key::XF86_MyComputer;
			if (key == "xf86.vendorHome") return Key::XF86_VendorHome;
			if (key == "xf86.lightBulb") return Key::XF86_LightBulb;
			if (key == "xf86.shop") return Key::XF86_Shop;
			if (key == "xf86.history") return Key::XF86_History;
			if (key == "xf86.openURL") return Key::XF86_OpenURL;
			if (key == "xf86.addFavorite") return Key::XF86_AddFavorite;
			if (key == "xf86.hotLinks") return Key::XF86_HotLinks;
			if (key == "xf86.brightnessAdjust") return Key::XF86_BrightnessAdjust;
			if (key == "xf86.finance") return Key::XF86_Finance;
			if (key == "xf86.community") return Key::XF86_Community;
			if (key == "xf86.audioRewind") return Key::XF86_AudioRewind;
			if (key == "xf86.backForward") return Key::XF86_BackForward;
			if (key == "xf86.launch0") return Key::XF86_Launch0;
			if (key == "xf86.launch1") return Key::XF86_Launch1;
			if (key == "xf86.launch2") return Key::XF86_Launch2;
			if (key == "xf86.launch3") return Key::XF86_Launch3;
			if (key == "xf86.launch4") return Key::XF86_Launch4;
			if (key == "xf86.launch5") return Key::XF86_Launch5;
			if (key == "xf86.launch6") return Key::XF86_Launch6;
			if (key == "xf86.launch7") return Key::XF86_Launch7;
			if (key == "xf86.launch8") return Key::XF86_Launch8;
			if (key == "xf86.launch9") return Key::XF86_Launch9;
			if (key == "xf86.launchA") return Key::XF86_LaunchA;
			if (key == "xf86.launchB") return Key::XF86_LaunchB;
			if (key == "xf86.launchC") return Key::XF86_LaunchC;
			if (key == "xf86.launchD") return Key::XF86_LaunchD;
			if (key == "xf86.launchE") return Key::XF86_LaunchE;
			if (key == "xf86.launchF") return Key::XF86_LaunchF;

			if (key == "xf86.applicationLeft") return Key::XF86_ApplicationLeft;
			if (key == "xf86.applicationRight") return Key::XF86_ApplicationRight;
			if (key == "xf86.book") return Key::XF86_Book;
			if (key == "xf86.cd") return Key::XF86_CD;
			if (key == "xf86.mediaSelectCD") return Key::XF86_MediaSelectCD;
			if (key == "xf86.calculater") return Key::XF86_Calculater;
			if (key == "xf86.Clear") return Key::XF86_Clear;
			if (key == "xf86.close") return Key::XF86_Close;
			if (key == "xf86.copy") return Key::XF86_Copy;
			if (key == "xf86.cut") return Key::XF86_Cut;
			if (key == "xf86.display") return Key::XF86_Display;
			if (key == "xf86.dos") return Key::XF86_DOS;
			if (key == "xf86.documents") return Key::XF86_Documents;
			if (key == "xf86.excel") return Key::XF86_Excel;
			if (key == "xf86.explorer") return Key::XF86_Explorer;
			if (key == "xf86.game") return Key::XF86_Game;
			if (key == "xf86.go") return Key::XF86_Go;
			if (key == "xf86.iTouch") return Key::XF86_iTouch;
			if (key == "xf86.logOff") return Key::XF86_LogOff;
			if (key == "xf86.market") return Key::XF86_Market;
			if (key == "xf86.meeting") return Key::XF86_Meeting;
			if (key == "xf86.menuKB") return Key::XF86_MenuKB;
			if (key == "xf86.menuPB") return Key::XF86_MenuPB;
			if (key == "xf86.mySites") return Key::XF86_MySites;
			if (key == "xf86.new") return Key::XF86_New;
			if (key == "xf86.news") return Key::XF86_News;
			if (key == "xf86.officeHome") return Key::XF86_OfficeHome;
			if (key == "xf86.open") return Key::XF86_Open;
			if (key == "xf86.option") return Key::XF86_Option;
			if (key == "xf86.paste") return Key::XF86_Paste;
			if (key == "xf86.phone") return Key::XF86_Phone;
			if (key == "xf86.q") return Key::XF86_Q;
			if (key == "xf86.reply") return Key::XF86_Reply;
			if (key == "xf86.reload") return Key::XF86_Reload;
			if (key == "xf86.rotateWindows") return Key::XF86_RotateWindows;
			if (key == "xf86.rotationPB") return Key::XF86_RotationPB;
			if (key == "xf86.rotationKB") return Key::XF86_RotationKB;
			if (key == "xf86.save") return Key::XF86_Save;
			if (key == "xf86.scrollUp") return Key::XF86_ScrollUp;
			if (key == "xf86.scrollDown") return Key::XF86_ScrollDown;
			if (key == "xf86.scrollClick") return Key::XF86_ScrollClick;
			if (key == "xf86.send") return Key::XF86_Send;
			if (key == "xf86.spell") return Key::XF86_Spell;
			if (key == "xf86.splitScreen") return Key::XF86_SplitScreen;
			if (key == "xf86.support") return Key::XF86_Support;
			if (key == "xf86.taskPane") return Key::XF86_TaskPane;
			if (key == "xf86.terminal") return Key::XF86_Terminal;
			if (key == "xf86.tools") return Key::XF86_Tools;
			if (key == "xf86.travel") return Key::XF86_Travel;
			if (key == "xf86.userPB") return Key::XF86_UserPB;
			if (key == "xf86.user1KB") return Key::XF86_User1KB;
			if (key == "xf86.user2KB") return Key::XF86_User2KB;
			if (key == "xf86.video") return Key::XF86_Video;
			if (key == "xf86.wheelButton") return Key::XF86_WheelButton;
			if (key == "xf86.word") return Key::XF86_Word;
			if (key == "xf86.xfer") return Key::XF86_Xfer;
			if (key == "xf86.zoomIn") return Key::XF86_ZoomIn;
			if (key == "xf86.zoomOut") return Key::XF86_ZoomOut;

			if (key == "xf86.away") return Key::XF86_Away;
			if (key == "xf86.messenger") return Key::XF86_Messenger;
			if (key == "xf86.webCam") return Key::XF86_WebCam;
			if (key == "xf86.mailForward") return Key::XF86_MailForward;
			if (key == "xf86.pictures") return Key::XF86_Pictures;
			if (key == "xf86.music") return Key::XF86_Music;

			if (key == "xf86.battery") return Key::XF86_Battery;
			if (key == "xf86.bluetooth") return Key::XF86_Bluetooth;
			if (key == "xf86.wlan") return Key::XF86_WLAN;
			if (key == "xf86.uwb") return Key::XF86_UWB;

			if (key == "xf86.audioForward") return Key::XF86_AudioForward;
			if (key == "xf86.audioRepeat") return Key::XF86_AudioRepeat;
			if (key == "xf86.audioRandomPlay") return Key::XF86_AudioRandomPlay;
			if (key == "xf86.subtitle") return Key::XF86_Subtitle;
			if (key == "xf86.audioCycleTrack") return Key::XF86_AudioCycleTrack;
			if (key == "xf86.cycleAngle") return Key::XF86_CycleAngle;
			if (key == "xf86.frameBack") return Key::XF86_FrameBack;
			if (key == "xf86.frameForward") return Key::XF86_FrameForward;
			if (key == "xf86.time") return Key::XF86_Time;
			if (key == "xf86.select") return Key::XF86_Select;
			if (key == "xf86.view") return Key::XF86_View;
			if (key == "xf86.topMenu") return Key::XF86_TopMenu;

			if (key == "xf86.red") return Key::XF86_Red;
			if (key == "xf86.green") return Key::XF86_Green;
			if (key == "xf86.yellow") return Key::XF86_Yellow;
			if (key == "xf86.blue") return Key::XF86_Blue;

			if (key == "xf86.suspend") return Key::XF86_Suspend;
			if (key == "xf86.hibernate") return Key::XF86_Hibernate;
			if (key == "xf86.touchpadToggle") return Key::XF86_TouchpadToggle;
			if (key == "xf86.touchpadOn") return Key::XF86_TouchpadOn;
			if (key == "xf86.touchpadOff") return Key::XF86_TouchpadOff;

			if (key == "xf86.audioMicMute") return Key::XF86_AudioMicMute;
			if (key == "xf86.keyboard") return Key::XF86_Keyboard;

			if (key == "xf86.wwan") return Key::XF86_WWAN;
			if (key == "xf86.rfKill") return Key::XF86_RFKill;

			if (key == "xf86.audioPreset") return Key::XF86_AudioPreset;
			if (key == "xf86.rotationLockToggle") return Key::XF86_RotationLockToggle;
			if (key == "xf86.fullScreen") return Key::XF86_FullScreen;

			if (key == "xf86.switch_VT_1") return Key::XF86_Switch_VT_1;
			if (key == "xf86.switch_VT_2") return Key::XF86_Switch_VT_2;
			if (key == "xf86.switch_VT_3") return Key::XF86_Switch_VT_3;
			if (key == "xf86.switch_VT_4") return Key::XF86_Switch_VT_4;
			if (key == "xf86.switch_VT_5") return Key::XF86_Switch_VT_5;
			if (key == "xf86.switch_VT_6") return Key::XF86_Switch_VT_6;
			if (key == "xf86.switch_VT_7") return Key::XF86_Switch_VT_7;
			if (key == "xf86.switch_VT_8") return Key::XF86_Switch_VT_8;
			if (key == "xf86.switch_VT_9") return Key::XF86_Switch_VT_9;
			if (key == "xf86.switch_VT_10") return Key::XF86_Switch_VT_10;
			if (key == "xf86.switch_VT_11") return Key::XF86_Switch_VT_11;
			if (key == "xf86.switch_VT_12") return Key::XF86_Switch_VT_12;

			if (key == "xf86.ungrab") return Key::XF86_Ungrab;
			if (key == "xf86.ClearGrab") return Key::XF86_ClearGrab;
			if (key == "xf86.Next_VMode") return Key::XF86_Next_VMode;
			if (key == "xf86.prev_VMode") return Key::XF86_Prev_VMode;
			if (key == "xf86.logWindowTree") return Key::XF86_LogWindowTree;
			if (key == "xf86.logGrabInfo") return Key::XF86_LogGrabInfo;

			if (key == "xf86.mediaPlayPause") return Key::XF86_MediaPlayPause;
			if (key == "xf86.exit") return Key::XF86_Exit;
			if (key == "xf86.audioBassBoost") return Key::XF86_AudioBassBoost;
			if (key == "xf86.sport") return Key::XF86_Sport;
			if (key == "xf86.monBrightnessAuto") return Key::XF86_MonBrightnessAuto;
			if (key == "xf86.displayOff") return Key::XF86_DisplayOff;
			if (key == "xf86.ok") return Key::XF86_OK;
			if (key == "xf86.goTo") return Key::XF86_GoTo;
			if (key == "xf86.info") return Key::XF86_Info;
			if (key == "xf86.vendorLogo") return Key::XF86_VendorLogo;
			if (key == "xf86.mediaSelectProgramGuide") return Key::XF86_MediaSelectProgramGuide;
			if (key == "xf86.mediaSelectHome") return Key::XF86_MediaSelectHome;
			if (key == "xf86.mediaLanguageMenu") return Key::XF86_MediaLanguageMenu;
			if (key == "xf86.mediaTitleMenu") return Key::XF86_MediaTitleMenu;
			if (key == "xf86.audioChannelMode") return Key::XF86_AudioChannelMode;
			if (key == "xf86.AspectRatio") return Key::XF86_AspectRatio;
			if (key == "xf86.mediaSelectPC") return Key::XF86_MediaSelectPC;
			if (key == "xf86.mediaSelectTV") return Key::XF86_MediaSelectTV;
			if (key == "xf86.mediaSelectCable") return Key::XF86_MediaSelectCable;
			if (key == "xf86.mediaSelectVCR") return Key::XF86_MediaSelectVCR;
			if (key == "xf86.mediaSelectVCRPlus") return Key::XF86_MediaSelectVCRPlus;
			if (key == "xf86.mediaSelectSatellite") return Key::XF86_MediaSelectSatellite;
			if (key == "xf86.mediaSelectTape") return Key::XF86_MediaSelectTape;
			if (key == "xf86.mediaSelectRadio") return Key::XF86_MediaSelectRadio;
			if (key == "xf86.mediaSelectTuner") return Key::XF86_MediaSelectTuner;
			if (key == "xf86.mediaPlayer") return Key::XF86_MediaPlayer;
			if (key == "xf86.mediaSelectTeletext") return Key::XF86_MediaSelectTeletext;
			if (key == "xf86.dvd") return Key::XF86_DVD;
			if (key == "xf86.mediaSelectDVD") return Key::XF86_MediaSelectDVD;
			if (key == "xf86.mediaSelectAuxiliary") return Key::XF86_MediaSelectAuxiliary;
			if (key == "xf86.audio") return Key::XF86_Audio;
			if (key == "xf86.channelUp") return Key::XF86_ChannelUp;
			if (key == "xf86.channelDown") return Key::XF86_ChannelDown;
			if (key == "xf86.mediaPlaySlow") return Key::XF86_MediaPlaySlow;
			if (key == "xf86.break") return Key::XF86_Break;
			if (key == "xf86.numberEntryMode") return Key::XF86_NumberEntryMode;
			if (key == "xf86.videoPhone") return Key::XF86_VideoPhone;
			if (key == "xf86.zoomReset") return Key::XF86_ZoomReset;
			if (key == "xf86.editor") return Key::XF86_Editor;
			if (key == "xf86.graphicsEditor") return Key::XF86_GraphicsEditor;
			if (key == "xf86.presentation") return Key::XF86_Presentation;
			if (key == "xf86.database") return Key::XF86_Database;
			if (key == "xf86.voicemail") return Key::XF86_Voicemail;
			if (key == "xf86.addressbook") return Key::XF86_Addressbook;
			if (key == "xf86.displayToggle") return Key::XF86_DisplayToggle;
			if (key == "xf86.spellCheck") return Key::XF86_SpellCheck;
			if (key == "xf86.contextMenu") return Key::XF86_ContextMenu;
			if (key == "xf86.mediaRepeat") return Key::XF86_MediaRepeat;
			if (key == "xf86.10ChannelsUp") return Key::XF86_10ChannelsUp;
			if (key == "xf86.10ChannelsDown") return Key::XF86_10ChannelsDown;
			if (key == "xf86.images") return Key::XF86_Images;
			if (key == "xf86.notificationCenter") return Key::XF86_NotificationCenter;
			if (key == "xf86.pickupPhone") return Key::XF86_PickupPhone;
			if (key == "xf86.hangupPhone") return Key::XF86_HangupPhone;
			if (key == "xf86.linkPhone") return Key::XF86_LinkPhone;
			if (key == "xf86.fn") return Key::XF86_Fn;
			if (key == "xf86.fn_Esc") return Key::XF86_Fn_Esc;
			if (key == "xf86.fn_F1") return Key::XF86_Fn_F1;
			if (key == "xf86.fn_F2") return Key::XF86_Fn_F2;
			if (key == "xf86.fn_F3") return Key::XF86_Fn_F3;
			if (key == "xf86.fn_F4") return Key::XF86_Fn_F4;
			if (key == "xf86.fn_F5") return Key::XF86_Fn_F5;
			if (key == "xf86.fn_F6") return Key::XF86_Fn_F6;
			if (key == "xf86.fn_F7") return Key::XF86_Fn_F7;
			if (key == "xf86.fn_F8") return Key::XF86_Fn_F8;
			if (key == "xf86.fn_F9") return Key::XF86_Fn_F9;
			if (key == "xf86.fn_F10") return Key::XF86_Fn_F10;
			if (key == "xf86.fn_F11") return Key::XF86_Fn_F11;
			if (key == "xf86.fn_F12") return Key::XF86_Fn_F12;
			if (key == "xf86.fn_1") return Key::XF86_Fn_1;
			if (key == "xf86.fn_2") return Key::XF86_Fn_2;
			if (key == "xf86.fn_D") return Key::XF86_Fn_D;
			if (key == "xf86.fn_E") return Key::XF86_Fn_E;
			if (key == "xf86.fn_F") return Key::XF86_Fn_F;
			if (key == "xf86.fn_S") return Key::XF86_Fn_S;
			if (key == "xf86.fn_B") return Key::XF86_Fn_B;
			if (key == "xf86.fnRightShift") return Key::XF86_FnRightShift;
			if (key == "xf86.num0") return Key::XF86_Numeric0;
			if (key == "xf86.num1") return Key::XF86_Numeric1;
			if (key == "xf86.num2") return Key::XF86_Numeric2;
			if (key == "xf86.num3") return Key::XF86_Numeric3;
			if (key == "xf86.num4") return Key::XF86_Numeric4;
			if (key == "xf86.num5") return Key::XF86_Numeric5;
			if (key == "xf86.num6") return Key::XF86_Numeric6;
			if (key == "xf86.num7") return Key::XF86_Numeric7;
			if (key == "xf86.num8") return Key::XF86_Numeric8;
			if (key == "xf86.num9") return Key::XF86_Numeric9;
			if (key == "xf86.numStar") return Key::XF86_NumericStar;
			if (key == "xf86.numPound") return Key::XF86_NumericPound;
			if (key == "xf86.numA") return Key::XF86_NumericA;
			if (key == "xf86.numB") return Key::XF86_NumericB;
			if (key == "xf86.numC") return Key::XF86_NumericC;
			if (key == "xf86.numD") return Key::XF86_NumericD;
			if (key == "xf86.cameraFocus") return Key::XF86_CameraFocus;
			if (key == "xf86.wpsButton") return Key::XF86_WPSButton;
			if (key == "xf86.cameraZoomIn") return Key::XF86_CameraZoomIn;
			if (key == "xf86.cameraZoomOut") return Key::XF86_CameraZoomOut;
			if (key == "xf86.cameraUp") return Key::XF86_CameraUp;
			if (key == "xf86.cameraDown") return Key::XF86_CameraDown;
			if (key == "xf86.cameraLeft") return Key::XF86_CameraLeft;
			if (key == "xf86.cameraRight") return Key::XF86_CameraRight;
			if (key == "xf86.attendantOn") return Key::XF86_AttendantOn;
			if (key == "xf86.attendantOff") return Key::XF86_AttendantOff;
			if (key == "xf86.attendantToggle") return Key::XF86_AttendantToggle;
			if (key == "xf86.lightsToggle") return Key::XF86_LightsToggle;
			if (key == "xf86.aLSToggle") return Key::XF86_ALSToggle;
			if (key == "xf86.refreshRateToggle") return Key::XF86_RefreshRateToggle;
			if (key == "xf86.buttonconfig") return Key::XF86_Buttonconfig;
			if (key == "xf86.taskmanager") return Key::XF86_Taskmanager;
			if (key == "xf86.journal") return Key::XF86_Journal;
			if (key == "xf86.controlPanel") return Key::XF86_ControlPanel;
			if (key == "xf86.appSelect") return Key::XF86_AppSelect;
			if (key == "xf86.screensaver") return Key::XF86_Screensaver;
			if (key == "xf86.voiceCommand") return Key::XF86_VoiceCommand;
			if (key == "xf86.Assistant") return Key::XF86_Assistant;
			if (key == "xf86.emojiPicker") return Key::XF86_EmojiPicker;
			if (key == "xf86.dictate") return Key::XF86_Dictate;
			if (key == "xf86.cameraAccessEnable") return Key::XF86_CameraAccessEnable;
			if (key == "xf86.cameraAccessDisable") return Key::XF86_CameraAccessDisable;
			if (key == "xf86.cameraAccessToggle") return Key::XF86_CameraAccessToggle;
			if (key == "xf86.accessibility") return Key::XF86_Accessibility;
			if (key == "xf86.doNotDisturb") return Key::XF86_DoNotDisturb;
			if (key == "xf86.brightnessMin") return Key::XF86_BrightnessMin;
			if (key == "xf86.brightnessMax") return Key::XF86_BrightnessMax;
			if (key == "xf86.electronicPrivacyScreenOn") return Key::XF86_ElectronicPrivacyScreenOn;
			if (key == "xf86.electronicPrivacyScreenOff") return Key::XF86_ElectronicPrivacyScreenOff;
			if (key == "xf86.actionOnSelection") return Key::XF86_ActionOnSelection;
			if (key == "xf86.contextualInsert") return Key::XF86_ContextualInsert;
			if (key == "xf86.contextualQuery") return Key::XF86_ContextualQuery;
			if (key == "xf86.kbdInputAssistPrev") return Key::XF86_KbdInputAssistPrev;
			if (key == "xf86.kbdInputAssistNext") return Key::XF86_KbdInputAssistNext;
			if (key == "xf86.kbdInputAssistPrevgroup") return Key::XF86_KbdInputAssistPrevgroup;
			if (key == "xf86.kbdInputAssistNextgroup") return Key::XF86_KbdInputAssistNextgroup;
			if (key == "xf86.kbdInputAssistAccept") return Key::XF86_KbdInputAssistAccept;
			if (key == "xf86.kbdInputAssistCancel") return Key::XF86_KbdInputAssistCancel;
			if (key == "xf86.rightUp") return Key::XF86_RightUp;
			if (key == "xf86.rightDown") return Key::XF86_RightDown;
			if (key == "xf86.leftUp") return Key::XF86_LeftUp;
			if (key == "xf86.leftDown") return Key::XF86_LeftDown;
			if (key == "xf86.rootMenu") return Key::XF86_RootMenu;
			if (key == "xf86.mediaTopMenu") return Key::XF86_MediaTopMenu;
			if (key == "xf86.num11") return Key::XF86_Numeric11;
			if (key == "xf86.num12") return Key::XF86_Numeric12;
			if (key == "xf86.audioDesc") return Key::XF86_AudioDesc;
			if (key == "xf86.3DMode") return Key::XF86_3DMode;
			if (key == "xf86.NextFavorite") return Key::XF86_NextFavorite;
			if (key == "xf86.stopRecord") return Key::XF86_StopRecord;
			if (key == "xf86.pauseRecord") return Key::XF86_PauseRecord;
			if (key == "xf86.vOD") return Key::XF86_VOD;
			if (key == "xf86.unmute") return Key::XF86_Unmute;
			if (key == "xf86.fastReverse") return Key::XF86_FastReverse;
			if (key == "xf86.slowReverse") return Key::XF86_SlowReverse;
			if (key == "xf86.data") return Key::XF86_Data;
			if (key == "xf86.onScreenKeyboard") return Key::XF86_OnScreenKeyboard;
			if (key == "xf86.privacyScreenToggle") return Key::XF86_PrivacyScreenToggle;
			if (key == "xf86.selectiveScreenshot") return Key::XF86_SelectiveScreenshot;
			if (key == "xf86.NextElement") return Key::XF86_NextElement;
			if (key == "xf86.previousElement") return Key::XF86_PreviousElement;
			if (key == "xf86.autopilotEngageToggle") return Key::XF86_AutopilotEngageToggle;
			if (key == "xf86.markWaypoint") return Key::XF86_MarkWaypoint;
			if (key == "xf86.sos") return Key::XF86_Sos;
			if (key == "xf86.navChart") return Key::XF86_NavChart;
			if (key == "xf86.fishingChart") return Key::XF86_FishingChart;
			if (key == "xf86.singleRangeRadar") return Key::XF86_SingleRangeRadar;
			if (key == "xf86.dualRangeRadar") return Key::XF86_DualRangeRadar;
			if (key == "xf86.radarOverlay") return Key::XF86_RadarOverlay;
			if (key == "xf86.traditionalSonar") return Key::XF86_TraditionalSonar;
			if (key == "xf86.ClearvuSonar") return Key::XF86_ClearvuSonar;
			if (key == "xf86.sidevuSonar") return Key::XF86_SidevuSonar;
			if (key == "xf86.navInfo") return Key::XF86_NavInfo;
			if (key == "xf86.macro1") return Key::XF86_Macro1;
			if (key == "xf86.macro2") return Key::XF86_Macro2;
			if (key == "xf86.macro3") return Key::XF86_Macro3;
			if (key == "xf86.macro4") return Key::XF86_Macro4;
			if (key == "xf86.macro5") return Key::XF86_Macro5;
			if (key == "xf86.macro6") return Key::XF86_Macro6;
			if (key == "xf86.macro7") return Key::XF86_Macro7;
			if (key == "xf86.macro8") return Key::XF86_Macro8;
			if (key == "xf86.macro9") return Key::XF86_Macro9;
			if (key == "xf86.macro10") return Key::XF86_Macro10;
			if (key == "xf86.macro11") return Key::XF86_Macro11;
			if (key == "xf86.macro12") return Key::XF86_Macro12;
			if (key == "xf86.macro13") return Key::XF86_Macro13;
			if (key == "xf86.macro14") return Key::XF86_Macro14;
			if (key == "xf86.macro15") return Key::XF86_Macro15;
			if (key == "xf86.macro16") return Key::XF86_Macro16;
			if (key == "xf86.macro17") return Key::XF86_Macro17;
			if (key == "xf86.macro18") return Key::XF86_Macro18;
			if (key == "xf86.macro19") return Key::XF86_Macro19;
			if (key == "xf86.macro20") return Key::XF86_Macro20;
			if (key == "xf86.macro21") return Key::XF86_Macro21;
			if (key == "xf86.macro22") return Key::XF86_Macro22;
			if (key == "xf86.macro23") return Key::XF86_Macro23;
			if (key == "xf86.macro24") return Key::XF86_Macro24;
			if (key == "xf86.macro25") return Key::XF86_Macro25;
			if (key == "xf86.macro26") return Key::XF86_Macro26;
			if (key == "xf86.macro27") return Key::XF86_Macro27;
			if (key == "xf86.macro28") return Key::XF86_Macro28;
			if (key == "xf86.macro29") return Key::XF86_Macro29;
			if (key == "xf86.macro30") return Key::XF86_Macro30;
			if (key == "xf86.macroRecordStart") return Key::XF86_MacroRecordStart;
			if (key == "xf86.macroRecordStop") return Key::XF86_MacroRecordStop;
			if (key == "xf86.macroPresetCycle") return Key::XF86_MacroPresetCycle;
			if (key == "xf86.macroPreset1") return Key::XF86_MacroPreset1;
			if (key == "xf86.macroPreset2") return Key::XF86_MacroPreset2;
			if (key == "xf86.macroPreset3") return Key::XF86_MacroPreset3;
			if (key == "xf86.kbdLcdMenu1") return Key::XF86_KbdLcdMenu1;
			if (key == "xf86.kbdLcdMenu2") return Key::XF86_KbdLcdMenu2;
			if (key == "xf86.kbdLcdMenu3") return Key::XF86_KbdLcdMenu3;
			if (key == "xf86.kbdLcdMenu4") return Key::XF86_KbdLcdMenu4;
			if (key == "xf86.kbdLcdMenu5") return Key::XF86_KbdLcdMenu5;
			if (key == "xf86.performanceMode") return Key::XF86_PerformanceMode;

			return Key::None;
		};

		ModKey stringToModKey(Wx::String const& modKey) {
			if (modKey == "shift") return ModKey::Shift;
			if (modKey == "ctrl") return ModKey::Ctrl;
			if (modKey == "alt") return ModKey::Alt;
			if (modKey == "super") return ModKey::Super;
			return ModKey::None;
		}

		void parseAutostartList(Wx::WkValue const& value) {
			if (!value.IsList()) return;

			Wx::List<Wx::WkValue> const& list = value.AsList();
			for (int i = 0; i < list.Count(); i++) {
				if (!list[i].IsTuple()) {
					continue;
				}
				ConfigCommand command = parseCommand(list[i].AsTuple());
				if (command.func == ConfigCommandFunc::None) {
					continue;
				}
				config.autostarts.Append(command);
			}
		}
	};

	Parser parser;

	parser.parse(value);

	return parser.config;
}
