//  
//  Copyright (C) 2009 GNOME Do
// 
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
// 
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// 

using System;
using System.Collections.Generic;
using System.Linq;

using Mono.Unix;

using Gnome;
using AppIndicator;

namespace Tomboy
{
	public class TomboyIndicatorTray
	{
		public static bool IndicatorTrayPresent {
			get {
				return true;
			}
		}
		
		ApplicationIndicator indicator;
		NoteManager manager;
		Gtk.Menu menu;
		int idle_source = 0;

		public TomboyIndicatorTray (NoteManager manager)
		{
			this.manager = manager;
			indicator = new ApplicationIndicator ("tomboy-notes", "tomboy", Category.ApplicationStatus);
			
			SetMenuItems ();
			indicator.Status = Status.Active;
			indicator.Title = Catalog.GetString ("Tomboy Notes");

			manager.NoteDeleted += OnNoteDeleted;
			manager.NoteAdded += OnNoteAdded;
			manager.NoteRenamed += OnNoteRenamed;
			manager.NotesLoaded += OnNotesLoaded;
		}

		void OnNoteAdded (object sender, Note added)
		{
			SetMenuItems ();
		}

		void OnNoteDeleted (object sender, Note deleted)
		{
			SetMenuItems ();
		}

		void OnNoteRenamed (Note renamed, string old_title)
		{
			SetMenuItems ();
		}

		void OnNotesLoaded (object sender, EventArgs args)
		{
			SetMenuItems ();
		}
		
		void SetMenuItems ()
		{
			if (idle_source == 0)
				GLib.Idle.Add(SetMenuItemsIdle);
		}

		bool SetMenuItemsIdle ()
		{
			idle_source = 0;

			if (menu != null) {
				foreach (Gtk.Widget widget in menu.Children) {
					menu.Remove (widget);
					widget.Destroy ();
				}
			}
			
			menu = new Gtk.Menu ();
			
			foreach (Gtk.MenuItem item in CurrentMenuItems ()) {
				menu.Append (item);
				item.Show();
			}

			menu.Show();
			
			//if (indicator.Menu == null)
			indicator.Menu = menu;

			return false;
		}
		
		Gtk.MenuItem CreateNoteMenuItem(Note n)
		{
		    var item = new Gtk.MenuItem (n.Title);
			item.Activated += (o, a) => SetMenuItems ();
			item.Activated += (o, a) => n.Window.Present ();
			return item;
		}

		IEnumerable<Gtk.MenuItem> CurrentMenuItems ()
		{
			Gtk.ImageMenuItem item;
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("Create New Note"));
			item.Image = new Gtk.Image (Gtk.Stock.New, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["NewNoteAction"].Activate ();
			item.Activated += (o, a) => SetMenuItems ();
			yield return item;
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("_Search All Notes"));
			item.Image = new Gtk.Image (Gtk.Stock.Find, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["ShowSearchAllNotesAction"].Activate ();
			yield return item;
			
			yield return new Gtk.SeparatorMenuItem ();
			
			if (manager != null && manager.Notes != null) {
				Tag template_tag = TagManager.GetOrCreateSystemTag (TagManager.TemplateNoteSystemTag);
				var menuItems = manager.Notes
					.Where (n => !n.IsSpecial && !n.ContainsTag(template_tag))
					.OrderByDescending (n => n.ChangeDate)
					.Take (10)
					.Select (n => CreateNoteMenuItem(n))
					.ToArray ();
				// avoid lazy eval for menu item construction
				
				foreach (Gtk.MenuItem mi in menuItems) {
					yield return mi;
				}
			}
			
			yield return new Gtk.SeparatorMenuItem ();
			
			Gtk.MenuItem mitem = new Gtk.MenuItem (Catalog.GetString ("S_ynchronize Notes"));
			// setting this changes the menu text to "Convert"
			// item.Image = new Gtk.Image (Gtk.Stock.Convert, Gtk.IconSize.Menu);
			mitem.Activated += (o, a) => Tomboy.ActionManager["NoteSynchronizationAction"].Activate ();
			yield return mitem;
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("_Preferences"));
			item.Image = new Gtk.Image (Gtk.Stock.Preferences, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["ShowPreferencesAction"].Activate ();
			yield return item;
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("_Help"));
			item.Image = new Gtk.Image (Gtk.Stock.Help, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["ShowHelpAction"].Activate ();
			yield return item;
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("_About Tomboy"));
			item.Image = new Gtk.Image (Gtk.Stock.About, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["ShowAboutAction"].Activate ();
			yield return item;
			
			yield return new Gtk.SeparatorMenuItem ();
			
			item = new Gtk.ImageMenuItem (Catalog.GetString ("_Quit"));
			item.Image = new Gtk.Image (Gtk.Stock.Quit, Gtk.IconSize.Menu);
			item.Activated += (o, a) => Tomboy.ActionManager["QuitTomboyAction"].Activate ();
			yield return item;
		}
	}
}
