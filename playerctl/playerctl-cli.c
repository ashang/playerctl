/* vim:ts=2:sw=2:expandtab
 *
 * This file is part of playerctl.
 *
 * playerctl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * playerctl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with playerctl If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright © 2014, Tony Crisci
 */

#include "playerctl-player.h"

static char *player_name = NULL;
static char **command = NULL;

static char *description = "Available Commands:"
"\n  play \t\t\tCommand the player to play"
"\n  pause \t\tCommand the player to pause"
"\n  play-pause \t\tCommand the player to toggle between play/pause"
"\n  next \t\t\tCommand the player to skip to the next track"
"\n  previous \t\tCommand the player to skip to the previous track"
"\n  volume [LEVEL] \tPrint or set the volume to LEVEL from 0.0 to 1.0"
"\n  status \t\tGet the play status of the player"
"\n  metadata \t\tPrint metadata information for the current track";

static char *summary = "  For true players only: spotify, vlc, audacious, bmp, xmms2, mplayer, and others.";

static GOptionEntry entries[] = {
  { "player", 'p', 0, G_OPTION_ARG_STRING, &player_name, "The name of the player to control (default: the first available player)", "NAME" },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &command, NULL, "COMMAND" },
  { NULL }
};

int main (int argc, char *argv[])
{
  GOptionContext *context = NULL;
  GError *error = NULL;

  context = g_option_context_new("- Controller for MPRIS players");
  g_option_context_add_main_entries(context, entries, NULL);
  g_option_context_set_description(context, description);
  g_option_context_set_summary(context, summary);

  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("Option parsing failed: %s\n", error->message);
    return 1;
  }

  if (command == NULL) {
    g_print(g_option_context_get_help(context, TRUE, NULL));
    return 0;
  }

  PlayerctlPlayer *player = playerctl_player_new(player_name, &error);

  if (error != NULL) {
    g_print("Connection to player failed: %s\n", error->message);
    return 1;
  }

  if (g_strcmp0(command[0], "volume") == 0) {
    /* VOLUME */
    gdouble level;

    if (command[1]) {
      /* set */
      level = g_ascii_strtod(command[1], NULL);
      g_object_set(player, "volume", level, NULL);
    } else {
      /* get */
      g_object_get(player, "volume", &level, NULL);
      g_print("%g\n", level);
    }
  } else if (g_strcmp0(command[0], "play") == 0) {
    /* PLAY */
    playerctl_player_play(player, &error);
  } else if (g_strcmp0(command[0], "pause") == 0) {
    /* PAUSE */
    playerctl_player_pause(player, &error);
  } else if (g_strcmp0(command[0], "play-pause") == 0) {
    /* PLAY-PAUSE */
    playerctl_player_play_pause(player, &error);
  } else if (g_strcmp0(command[0], "next") == 0) {
    /* NEXT */
    playerctl_player_next(player, &error);
  } else if (g_strcmp0(command[0], "previous") == 0) {
    /* PREVIOUS */
    playerctl_player_previous(player, &error);
  } else if (g_strcmp0(command[0], "metadata") == 0) {
    /* METADATA */
    GVariant *metadata = NULL;
    g_object_get(player, "metadata", &metadata, NULL);

    if (metadata) {
      g_print("%s\n", g_variant_print(metadata, FALSE));
      g_variant_unref(metadata);
    } else {
      g_print("None\n");
    }
  } else if (g_strcmp0(command[0], "status") == 0) {
    /* STATUS */
    gchar *status = NULL;
    g_object_get(player, "status", &status, NULL);

    if (status) {
      g_print("%s\n", status);
    } else {
      g_print("Not available\n");
    }
  } else {
    /* unrecognized command */
    g_print(g_option_context_get_help(context, TRUE, NULL));
  }

  return 0;
}
