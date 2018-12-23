/*
 * dprpwg: a Deterministic Pseudo-Random PassWord Generator
 * Copyright (c) 2018 Jean-Baptiste HERVE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* This is a GTK client implementation for dprpwg.
 * It should build with either GTK2 or GTK3 if I did not mess up. */

#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "dprpwg_lib.h"

/* We do not use all parameters of GTK callbacks */
#define UNUSED_PARAM(Param) ((void) Param)

/* ---- Internal function declarations ---- */

/* Callback called when program is terminated */
static void cb_destroy(GtkWidget *widget, gpointer data);

/* Callback called when password needs to be generated */
static void cb_generate(GtkWidget *widget, gpointer data);

/* Callback called when the "fixed size" is ticked, to enable the size input */
static void cb_fixedsize_changed(GtkWidget *widget, gpointer data);

/* Function to fill the program window */
static void window_fill(GtkWidget *window);

/* Ugly function to clear the internal text input buffers */
static void clean_entry_buffer(GtkEntry *gtk_entry);

/* Check both master password input, then display a status message */
static int check_password_entries(GtkWidget *text_passwd,
                                  GtkWidget *text_passwd_check,
                                  GtkWidget *label_passwd_status);

/* All a bunch of widget that must be consulted when a password is to
 * be generated (note: nearly all widgets...) */
typedef struct {
  GtkWidget *text_origpasswd;
  GtkWidget *text_origpasswd_check;
  GtkWidget *label_origpasswd_status;
  GtkWidget *text_domain;
  GtkWidget *text_year;
  GtkWidget *text_newpasswd;
  GtkWidget *text_fixed_size;
  GtkWidget *label_entropy;
  GtkWidget *check_low_avail;
  GtkWidget *check_upp_avail;
  GtkWidget *check_dig_avail;
  GtkWidget *check_sym_avail;
  GtkWidget *security_icons[3];
} s_generate_data;

void clean_entry_buffer(GtkEntry *gtk_entry)
{
  /*
   * GtkEntry internal buffer cleaning; warning: a bit dirty!
   * Get the internal widget buffer, and memset() it.
   */
  char* buffer = NULL;
  size_t size = 0;

  buffer = (char*) gtk_entry_get_text(gtk_entry);
  size = gtk_entry_get_text_length(gtk_entry) * sizeof(gchar);

  memset(buffer, 0, size);
}

/* Application termination callback */
void cb_destroy(GtkWidget *widget, gpointer data)
{
  UNUSED_PARAM(widget);

  /* Get the pointers to all widgets */
  s_generate_data *generate_data = (s_generate_data *) data;

  /* Clean the input/output buffers that need it (i.e. password entries) */
  clean_entry_buffer(GTK_ENTRY(generate_data->text_newpasswd));
  clean_entry_buffer(GTK_ENTRY(generate_data->text_origpasswd));
  clean_entry_buffer(GTK_ENTRY(generate_data->text_origpasswd_check));

  /* Quit GTK */
  gtk_main_quit();
}

/* Callback called when the "fixed size" tick changes state */
void cb_fixedsize_changed(GtkWidget *widget, gpointer data)
{
  int state;
  GtkWidget *text_fixed_size = (GtkWidget *) data;
  GtkWidget *check_fixed_size = widget;

  /* Enable the "Fixed size" text input if the "fixed size" button is ticked */
  state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fixed_size));
  gtk_widget_set_sensitive(text_fixed_size, state);
}

/* Check both master password input, then display a status message */
static int check_password_entries(GtkWidget *text_passwd,
                                  GtkWidget *text_passwd_check,
                                  GtkWidget *label_passwd_status)
{
  const char* passwd = NULL;
  const char* passwd_check = NULL;

  passwd = gtk_entry_get_text(GTK_ENTRY(text_passwd));
  passwd_check = gtk_entry_get_text(GTK_ENTRY(text_passwd_check));

  if (strlen(passwd) == 0 || strlen(passwd_check) == 0) {
    gtk_label_set_text(GTK_LABEL(label_passwd_status), " ");
    return FALSE;
  } else if (!strcmp(passwd, passwd_check)) {
    gtk_label_set_markup(GTK_LABEL(label_passwd_status),
                         "<span foreground=\"green\">Identical passwords</span>");
    return TRUE;
  } else {
    gtk_label_set_markup(GTK_LABEL(label_passwd_status),
                         "<span foreground=\"red\" weight=\"bold\">Password mismatch!</span>");
    return FALSE;
  }
}

/* Password generation callback */
void cb_generate(GtkWidget *widget, gpointer data)
{
  s_generate_data *generate_data = NULL;
  const char *passwd = NULL;
  const char *domain = NULL;
  const char *year = NULL;
  char *new_passwd = NULL;
  char *password_strength_str;
  double password_strength;
  unsigned int flags;
  size_t fixed_size = 0;

  UNUSED_PARAM(widget);

  /* Get the pointers to all widgets */
  generate_data = (s_generate_data *) data;

  /* Hide all funny icons by default */
  gtk_widget_hide(generate_data->security_icons[0]);
  gtk_widget_hide(generate_data->security_icons[1]);
  gtk_widget_hide(generate_data->security_icons[2]);

  /* First, check both master password input match */
  if (!check_password_entries(generate_data->text_origpasswd,
                              generate_data->text_origpasswd_check,
                              generate_data->label_origpasswd_status)) {
    /* Nope, mismatch. Generate nothing */
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(generate_data->label_entropy), "N/A");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(generate_data->label_entropy), 0);
    gtk_entry_set_text(GTK_ENTRY(generate_data->text_newpasswd), "");
    gtk_widget_show(generate_data->security_icons[0]);
    return;
  }

  /* Get the input text entries */
  passwd = gtk_entry_get_text(GTK_ENTRY(generate_data->text_origpasswd));
  domain = gtk_entry_get_text(GTK_ENTRY(generate_data->text_domain));
  year = gtk_entry_get_text(GTK_ENTRY(generate_data->text_year));

  /* Get the fixed size input if the fixed size option is enabled */
  if (gtk_widget_get_sensitive(generate_data->text_fixed_size)) {
    fixed_size = (unsigned int) atoi(gtk_entry_get_text(GTK_ENTRY(generate_data->text_fixed_size)));
  }

  /* Generate the symbol category flags */
  flags = 0;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(generate_data->check_low_avail))) {
    flags |= FLAG_LOW_AVAIL;
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(generate_data->check_upp_avail))) {
    flags |= FLAG_UPP_AVAIL;
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(generate_data->check_dig_avail))) {
    flags |= FLAG_DIG_AVAIL;
  }

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(generate_data->check_sym_avail))) {
    flags |= FLAG_SYM_AVAIL;
  }

  /* Generate the password */
  generate_password(passwd, domain, year, fixed_size, &new_passwd, flags);

  /* Display the new password */
  gtk_entry_set_text(GTK_ENTRY(generate_data->text_newpasswd), new_passwd);

  /* Get password strength for display */
  password_strength = get_password_strength(new_passwd, (unsigned int) atoi(year), flags);

  /* TODO: Put this part elswhere. Or at least remove all the magic numbers */
  if (password_strength < 0.25) {
    password_strength_str = "Strength: ridiculously low";
    gtk_widget_show(generate_data->security_icons[0]);
  } else if (password_strength < 0.375) {
    password_strength_str = "Strength: very low";
    gtk_widget_show(generate_data->security_icons[0]);
  } else if (password_strength < 0.5) {
    password_strength_str = "Strength: low";
    gtk_widget_show(generate_data->security_icons[0]);
  } else if (password_strength < 0.625) {
    password_strength_str = "Strength: fair";
    gtk_widget_show(generate_data->security_icons[0]);
  } else if (password_strength < 0.75) {
    password_strength_str = "Strength: good";
    gtk_widget_show(generate_data->security_icons[1]);
  } else if (password_strength < 0.875) {
    password_strength_str = "Strength: great";
    gtk_widget_show(generate_data->security_icons[1]);
  } else if (password_strength < 1) {
    password_strength_str = "Strength: excellent";
    gtk_widget_show(generate_data->security_icons[2]);
  } else {
    password_strength_str = "Strength: overkill";
    gtk_widget_show(generate_data->security_icons[2]);
  }

  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(generate_data->label_entropy), password_strength_str);

  if (password_strength > 1) {
    password_strength = 1;
  } else if (password_strength < 0) {
    password_strength = 0;
  }

  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(generate_data->label_entropy), password_strength);

  /* Cleanup */
  memset(new_passwd, 0, strlen(new_passwd));
  free(new_passwd);
}

/* Main window filling and callback attaching */
void window_fill(GtkWidget* window)
{
  /* Lots of widgets */
  GtkWidget* table_global = NULL;

  GtkWidget* label_origpasswd = NULL;
  GtkWidget* text_origpasswd = NULL;

  GtkWidget* label_origpasswd_check = NULL;
  GtkWidget* text_origpasswd_check = NULL;

  GtkWidget* label_origpasswd_status = NULL;

  GtkWidget* label_domain = NULL;
  GtkWidget* text_domain = NULL;

  GtkWidget* label_year = NULL;
  GtkWidget* text_year = NULL;

  GtkWidget* label_newpasswd = NULL;
  GtkWidget* text_newpasswd = NULL;

  GtkWidget* check_low_avail = NULL;
  GtkWidget* check_upp_avail = NULL;
  GtkWidget* check_dig_avail = NULL;
  GtkWidget* check_sym_avail = NULL;

  GtkWidget* check_fixed_size = NULL;
  GtkWidget* text_fixed_size = NULL;

  GtkWidget* hseparator = NULL;

  GtkWidget* box_security = NULL;
  GtkWidget* label_entropy = NULL;
  GtkWidget* icon_security_low = NULL;
  GtkWidget* icon_security_med = NULL;
  GtkWidget* icon_security_high = NULL;

  /* Needed to save all widget pointers for callbacks */
  s_generate_data *generate_data = NULL;

  /* Needed variables to get the current year */
  struct tm* time_data;
  time_t time_value;

  /* Global table to put all the other widgets */
  table_global = gtk_table_new(13, 2, FALSE);
  gtk_table_set_row_spacings(GTK_TABLE(table_global), 3);
  gtk_table_set_col_spacings(GTK_TABLE(table_global), 3);

  /* Master password text input */
  label_origpasswd = gtk_label_new("Master password:");
  text_origpasswd = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(text_origpasswd), FALSE);
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_origpasswd, 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_origpasswd, 1, 2, 0, 1);

  /* Master password check text input */
  label_origpasswd_check = gtk_label_new("Re-enter password:");
  text_origpasswd_check = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(text_origpasswd_check), FALSE);
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_origpasswd_check, 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_origpasswd_check, 1, 2, 1, 2);

  /* Master password check status */
  label_origpasswd_status = gtk_label_new(" ");
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_origpasswd_status, 0, 2, 2, 3);

  /* Domain name text input */
  label_domain = gtk_label_new("Domain:");
  text_domain = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(text_domain), TRUE);
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_domain, 0, 1, 3, 4);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_domain, 1, 2, 3, 4);

  /* Year input selector */
  label_year = gtk_label_new("Year:");
  text_year = gtk_spin_button_new_with_range(2000, 3000, 1);
  gtk_spin_button_set_increments(GTK_SPIN_BUTTON(text_year), 1, 1);
  gtk_spin_button_set_digits(GTK_SPIN_BUTTON(text_year), 0);
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_year, 0, 1, 4, 5);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_year, 1, 2, 4, 5);

  /* By default, set year input to the current year */
  time_value = time(NULL);
  time_data = localtime(&time_value);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(text_year), time_data->tm_year + 1900);

  /* New password text output */
  label_newpasswd = gtk_label_new("Generated password:");
  text_newpasswd = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(text_newpasswd), TRUE);
  gtk_editable_set_editable(GTK_EDITABLE(text_newpasswd), FALSE);
  gtk_table_attach_defaults(GTK_TABLE(table_global), label_newpasswd, 0, 1, 11, 12);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_newpasswd, 1, 2, 11, 12);

  /* Checkboxes to configure the output symbol categories */
  check_low_avail = gtk_check_button_new_with_label("Lower case letters");
  check_upp_avail = gtk_check_button_new_with_label("Upper case letters");
  check_dig_avail = gtk_check_button_new_with_label("Digits");
  check_sym_avail = gtk_check_button_new_with_label("Symbols");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_low_avail), TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_upp_avail), TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_dig_avail), TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_sym_avail), TRUE);

  /* Add checkboxes to the global table */
  gtk_table_attach_defaults(GTK_TABLE(table_global), check_low_avail, 0, 2, 5, 6);
  gtk_table_attach_defaults(GTK_TABLE(table_global), check_upp_avail, 0, 2, 6, 7);
  gtk_table_attach_defaults(GTK_TABLE(table_global), check_dig_avail, 0, 2, 7, 8);
  gtk_table_attach_defaults(GTK_TABLE(table_global), check_sym_avail, 0, 2, 8, 9);

  /* Fixed size, if needed */
  check_fixed_size = gtk_check_button_new_with_label("Fixed size:");
  text_fixed_size = gtk_spin_button_new_with_range(1, 256, 1);
  gtk_spin_button_set_increments(GTK_SPIN_BUTTON(text_fixed_size), 1, 1);
  gtk_spin_button_set_digits(GTK_SPIN_BUTTON(text_fixed_size), 0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(text_fixed_size), 8);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fixed_size), FALSE);
  gtk_widget_set_sensitive(text_fixed_size, FALSE);
  /* Register a callback, to enable the input only when the box is checked */
  g_signal_connect(check_fixed_size, "clicked", G_CALLBACK(cb_fixedsize_changed), (void*) text_fixed_size);
  gtk_table_attach_defaults(GTK_TABLE(table_global), check_fixed_size, 0, 1, 9, 10);
  gtk_table_attach_defaults(GTK_TABLE(table_global), text_fixed_size, 1, 2, 9, 10);

  /* Horizontal separator */
  hseparator = gtk_hseparator_new();
  gtk_table_attach_defaults(GTK_TABLE(table_global), hseparator, 0, 2, 10, 11);

  /* Progress bar to display the password strength */
  label_entropy = gtk_progress_bar_new();

  /* "Funny" icons */
  {
    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    GdkPixbuf *pixbuf_security_low = gtk_icon_theme_load_icon(icon_theme,
                                     "security-low",
                                     24, /* size */
                                     0, NULL);
    GdkPixbuf *pixbuf_security_med = gtk_icon_theme_load_icon(icon_theme,
                                     "security-medium",
                                     24, /* size */
                                     0, NULL);
    GdkPixbuf *pixbuf_security_high = gtk_icon_theme_load_icon(icon_theme,
                                      "security-high",
                                      24, /* size */
                                      0, NULL);

    icon_security_low = gtk_image_new_from_pixbuf(pixbuf_security_low);
    icon_security_med = gtk_image_new_from_pixbuf(pixbuf_security_med);
    icon_security_high = gtk_image_new_from_pixbuf(pixbuf_security_high);
  }

  /* Put the progress bar and icons in a horizontal bar */
  box_security = gtk_hbox_new(FALSE, 4);
  gtk_box_pack_start(GTK_BOX(box_security), label_entropy, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(box_security), icon_security_low, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_security), icon_security_med, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box_security), icon_security_high, FALSE, FALSE, 0);
  gtk_table_attach_defaults(GTK_TABLE(table_global), box_security, 0, 2, 12, 13);

  /* Add the global table to the main window */
  gtk_container_add(GTK_CONTAINER(window), table_global);

  /* Store all pointers in the structure that will be used by callbacks */
  generate_data = (s_generate_data *) calloc(1, sizeof(s_generate_data));
  generate_data->text_origpasswd = text_origpasswd;
  generate_data->text_origpasswd_check = text_origpasswd_check;
  generate_data->label_origpasswd_status = label_origpasswd_status;
  generate_data->text_domain = text_domain;
  generate_data->text_year = text_year;
  generate_data->text_newpasswd = text_newpasswd;
  generate_data->text_fixed_size = text_fixed_size;
  generate_data->label_entropy = label_entropy;
  generate_data->check_low_avail = check_low_avail;
  generate_data->check_upp_avail = check_upp_avail;
  generate_data->check_dig_avail = check_dig_avail;
  generate_data->check_sym_avail = check_sym_avail;
  generate_data->security_icons[0] = icon_security_low;
  generate_data->security_icons[1] = icon_security_med;
  generate_data->security_icons[2] = icon_security_high;

  /* Password generated upon text entry and button click */
  g_signal_connect(check_low_avail, "clicked", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(check_upp_avail, "clicked", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(check_dig_avail, "clicked", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(check_sym_avail, "clicked", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(check_fixed_size, "clicked", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(text_origpasswd, "changed", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(text_origpasswd_check, "changed", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(text_domain, "changed", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(text_year, "changed", G_CALLBACK(cb_generate), (void*) generate_data);
  g_signal_connect(text_fixed_size, "changed", G_CALLBACK(cb_generate), (void*) generate_data);

  /* Call it once to display the funny icon correctly */
  cb_generate(NULL, (void*) generate_data);

  /* "Destroy" callback used on program termination
   * Also give all the widget pointers, because we want to erase passwords
   * from memory before exit */
  g_signal_connect(window, "destroy", G_CALLBACK(cb_destroy), (void*)generate_data);

  /* Display all the widgets */
  gtk_widget_show(check_low_avail);
  gtk_widget_show(check_upp_avail);
  gtk_widget_show(check_dig_avail);
  gtk_widget_show(check_sym_avail);
  gtk_widget_show(label_origpasswd);
  gtk_widget_show(text_origpasswd);
  gtk_widget_show(label_origpasswd_check);
  gtk_widget_show(text_origpasswd_check);
  gtk_widget_show(label_origpasswd_status);
  gtk_widget_show(label_domain);
  gtk_widget_show(text_domain);
  gtk_widget_show(label_year);
  gtk_widget_show(text_year);
  gtk_widget_show(check_fixed_size);
  gtk_widget_show(text_fixed_size);
  gtk_widget_show(hseparator);
  gtk_widget_show(label_entropy);
  gtk_widget_show(label_newpasswd);
  gtk_widget_show(text_newpasswd);
  gtk_widget_show(table_global);
  gtk_widget_show(box_security);
  /* Funny icons are not diplayed here */
}


/* Useful function */
int main(int argc, char *argv[])
{
  GtkWidget* window = NULL; /* a GTK window is also useful for a GTK app */

  /* Init GTK */
  gtk_init(&argc, &argv);

  /* Instanciate the window */
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Deterministic Pseudo-Random PassWord Generator");

  /* Set window border width to a nicer value */
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);

  /* Now fill the window */
  window_fill(window);

  /* Set the window icon */
  gtk_window_set_icon_name(GTK_WINDOW(window), "dialog-password");

  /* Display the window */
  gtk_widget_show(window);

  /* We are up now, give hand to GTK */
  gtk_main();

  return 0;
}
