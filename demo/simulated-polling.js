// For simulation/demo purposes, speed up the tempo from the normal 10-second pause after animation
g_display_duration_after_animation = 5000;

function simulation_state() {
    if (g_overlay_shown == '' && 1 <= g_heat && g_heat <= 4) {
        // state must be numeric for switch statement to work
        return parseInt(g_heat);
    } else if (g_overlay_shown == '#timer_overlay') {
        return 5;
    } else if (g_overlay_shown == '#paused_overlay') {
        return 6;
    } else {
        return -1;
    }
}

var g_simulator_strings = {
    timer_not_connected: '<watching>\n' +
                    '<timer-not-connected/>\n' +
                    '<current-heat now-racing="0" use-master-sched="0" classid="5" roundid="5"' + 
                    ' round="1" group="5" heat="1" number-of-heats="4">Wolves</current-heat>\n' +
                    '</watching>',
    not_racing_heat_1: '<watching>\n' +
                    '<current-heat now-racing="0" use-master-sched="0" classid="5" roundid="5"' + 
                    ' round="1" group="5" heat="1" number-of-heats="4">Wolves</current-heat>\n' +
                    '</watching>',
    heat_1_ready: '<watching>\n' +
                    '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                    ' round="1" group="5" heat="1" number-of-heats="4">Wolves</current-heat>\n' +
                    '<racer lane="1" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                    '<racer lane="2" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                    '<racer lane="3" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                    '<racer lane="4" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                    '</watching>',
    heat_1_to_heat_2: '<watching>\n' +
                '<heat-result lane="1" time="2.729" place="3" speed="249.8"/>\n' +
                '<heat-result lane="2" time="2.378" place="1" speed="286.7"/>\n' +
                '<heat-result lane="3" time="2.458" place="2" speed="277.4"/>\n' +
                '<heat-result lane="4" time="3.076" place="4" speed="221.6"/>\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="2" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_2_ready: '<watching>\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="2" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_2_to_heat_3: '<watching>​\n' +
                '<heat-result lane="1" time="2.943" place="2" speed="231.7"/>\n' +
                '<heat-result lane="2" time="2.798" place="1" speed="243.7"/>\n' +
                '<heat-result lane="3" time="3.755" place="4" speed="181.6"/>\n' +
                '<heat-result lane="4" time="3.602" place="3" speed="189.3"/>\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="3" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_3_ready: '<watching>​\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="3" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_3_to_heat_4: '<watching>​\n' +
                '<heat-result lane="1" time="3.049" place="3" speed="223.6"/>\n' +
                '<heat-result lane="2" time="3.006" place="2" speed="226.8"/>\n' +
                '<heat-result lane="3" time="2.471" place="1" speed="276.0"/>\n' +
                '<heat-result lane="4" time="3.988" place="4" speed="170.9"/>\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="4" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_4_ready: '<watching>​\n' +
                '<current-heat now-racing="1" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="4" number-of-heats="4">Wolves</current-heat>\n' +
                '<racer lane="1" name="Julian Jarrard" carname="" carnumber="445" photo="" finishtime=""/>\n' +
                '<racer lane="2" name="Harley Howell" carname="" carnumber="435" photo="" finishtime=""/>\n' +
                '<racer lane="3" name="Earnest Evangelista" carname="" carnumber="425" photo="" finishtime=""/>\n' +
                '<racer lane="4" name="Cañumil Calero" carname="" carnumber="415" photo="" finishtime=""/>\n' +
                '</watching>',
    heat_4_done: '<watching>​\n' +
                '<heat-result lane="1" time="3.114" place="3" speed="218.9"/>\n' +
                '<heat-result lane="2" time="3.909" place="4" speed="174.4"/>\n' +
                '<heat-result lane="3" time="2.692" place="1" speed="253.3"/>\n' +
                '<heat-result lane="4" time="2.744" place="2" speed="248.5"/>\n' +
                '<current-heat now-racing="0" use-master-sched="0" classid="5" roundid="5"' +
                ' round="1" group="5" heat="4" number-of-heats="4">Wolves</current-heat>\n' +
                '</watching>',
};

var g_prior_state = -99;
var g_last_transition_time = 0;
function simulated_poll_string() {
    var state = simulation_state();
    var now = (new Date()).valueOf();
    if (state != g_prior_state) {
        g_prior_state = state;
        g_last_transition_time = now;
    }
    switch (state) {
        default:
        case 5:  // Showing timer overlay
            if (now < g_last_transition_time + 1000) {
                return g_simulator_strings.timer_not_connected;
            } else {
                // Should switch to a "not racing" state
                return g_simulator_strings.not_racing_heat_1;
            }
        case 6:  // Showing "Not racing" overlay
            if (now < g_last_transition_time + 1500) {
                return g_simulator_strings.not_racing_heat_1;
            } else {
                return g_simulator_strings.heat_1_ready;
            }
        case 1:
            if (now < g_last_transition_time + 2000) {
                return g_simulator_strings.heat_1_ready;
            } else {
                return g_simulator_strings.heat_1_to_heat_2;
            }
        case 2:
            if (now < g_last_transition_time + 2000) {
                return g_simulator_strings.heat_2_ready;
            } else {
                return g_simulator_strings.heat_2_to_heat_3;
            }
        case 3:
            if (now < g_last_transition_time + 2000) {
                return g_simulator_strings.heat_3_ready;
            } else {
                return g_simulator_strings.heat_3_to_heat_4;
            }
        case 4:
            if (now < g_last_transition_time + 2000) {
                return g_simulator_strings.heat_4_ready;
            } else {
                return g_simulator_strings.heat_4_done;
            }
    }
}

function simulated_poll_for_update() {
    process_watching($.parseXML(simulated_poll_string()));
}
