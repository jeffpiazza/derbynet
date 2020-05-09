local obs = obslua;

function script_properties()
  local props = obs.obs_properties_create()

  obs.obs_properties_add_path(props, "trigger_file_dir", "Trigger File Directory",
                              obs.OBS_PATH_DIRECTORY, "", "/tmp")
  obs.obs_properties_add_text(props, "heat_started_file", "Heat Started Trigger File",
                              obs.OBS_TEXT_DEFAULT)
  obs.obs_properties_add_text(props, "heat_finished_file", "Heat Finished Trigger File",
                              obs.OBS_TEXT_DEFAULT)

  obs.obs_properties_add_float_slider(props, "start_linger_sec", "Linger time on heat started",
                                    0, 10.0, 0.250)
  obs.obs_properties_add_float_slider(props, "dnf_limit_sec", "Time limit to assume DNF",
                                    0, 10.0, 0.250)
  obs.obs_properties_add_float_slider(props, "replay_duration_sec", "Replay time after heat finished",
                                    0, 30.0, 0.500)

  obs.obs_properties_add_text(props, "start_line_scene", "Starting line scene", obs.OBS_TEXT_DEFAULT)
  obs.obs_properties_add_text(props, "track_view_scene", "Track view scene", obs.OBS_TEXT_DEFAULT)
  -- Replay Scene is a Window running replay.php, not the OBS replay
  obs.obs_properties_add_text(props, "replay_scene", "Replay scene", obs.OBS_TEXT_DEFAULT)

  return props
end

function script_defaults(settings)
  obs.obs_data_set_default_string(settings, "trigger_file_dir", "/tmp")
  obs.obs_data_set_default_string(settings, "heat_started_file", "heat-started")
  obs.obs_data_set_default_string(settings, "heat_finished_file", "heat-finished")

  obs.obs_data_set_default_double(settings, "start_linger_sec", 1.0)
  obs.obs_data_set_default_double(settings, "dnf_limit_sec", 4.0)
  obs.obs_data_set_default_double(settings, "replay_duration_sec", 14.0)

  obs.obs_data_set_default_string(settings, "start_line_scene", "Start Line")
  obs.obs_data_set_default_string(settings, "track_view_scene", "Track View")
  obs.obs_data_set_default_string(settings, "replay_scene", "Replay Scene")
end

local TRIGGER_FILE_DIR
local HEAT_STARTED_FILE
local HEAT_FINISHED_FILE

local START_LINGER_MS
local DNF_LIMIT_MS
local REPLAY_DURATION_MS

local start_line_scene
local track_view_scene
local replay_scene

function script_update(settings)
  TRIGGER_FILE_DIR = obs.obs_data_get_string(settings, "trigger_file_dir")
  HEAT_STARTED_FILE =
      TRIGGER_FILE_DIR .. "/" .. obs.obs_data_get_string(settings, "heat_started_file")
  HEAT_FINISHED_FILE =
      TRIGGER_FILE_DIR .. "/" .. obs.obs_data_get_string(settings, "heat_finished_file")

  START_LINGER_MS = obs.obs_data_get_double(settings, "start_linger_sec") * 1000
  DNF_LIMIT_MS = obs.obs_data_get_double(settings, "dnf_limit_sec") * 1000
  REPLAY_DURATION_MS = obs.obs_data_get_double(settings, "replay_duration_sec") * 1000

  start_line_scene = find_scene_by_name(obs.obs_data_get_string(settings, "start_line_scene"))
  track_view_scene = find_scene_by_name(obs.obs_data_get_string(settings, "track_view_scene"))
  replay_scene = find_scene_by_name(obs.obs_data_get_string(settings, "replay_scene"))
end

function find_scene_by_name(name)
  local my_scene
  local scenes = obs.obs_frontend_get_scenes()
  for k, scene in pairs(scenes) do
    if obs.obs_source_get_name(scene) == name then
      my_scene = scene
    end
  end
  obs.source_list_release(scenes)
  return my_scene
end


function on_late_finish()
  obs.script_log(obs.LOG_INFO, string.format("on_late_finish: %s", os.date()))
  on_race_finished()
end

-- When the heat starts, continue to show the start line for a brief moment,
-- then switch to the track view.  Also start a timer to simulate a finish if a
-- real finish doesn't come.

function on_race_start()
  obs.script_log(obs.LOG_INFO, string.format("on_race_start: %s", os.date()))
  obs.timer_add(function()
      obs.script_log(obs.LOG_INFO, string.format("on_race_start linger: %s", os.date()))
      obs.remove_current_callback()
      obs.obs_frontend_set_current_scene(track_view_scene)
    end,
    START_LINGER_MS)

  obs.timer_add(on_late_finish, DNF_LIMIT_MS)
end

-- When the heat ends, switch to replay_scene, then after a few seconds, switch
-- to start_line_scene.

function on_race_finished()
  obs.script_log(obs.LOG_INFO, string.format("on_race_finished: %s", os.date()))
  obs.timer_remove(on_late_finish)
  obs.obs_frontend_set_current_scene(replay_scene)
  obs.timer_add(function()
    obs.script_log(obs.LOG_INFO,
                   string.format("on_race_finished after replay duration: %s",
                                 os.date()))
    obs.remove_current_callback()
    obs.obs_frontend_set_current_scene(start_line_scene)
    end,
    REPLAY_DURATION_MS)
end

-- Repeatedly poll for appearance of one of the trigger files

obs.timer_add(function()
  if os.remove(HEAT_STARTED_FILE) then
    on_race_start()
  elseif os.remove(HEAT_FINISHED_FILE) then
    on_race_finished()
  end
end,
200)

