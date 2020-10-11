import React, { useState } from 'react';
import logo from './logo.svg';
import './App.css';
import ESPHome, { useEventSource, useEventSourceListener } from './esphome/esphome-client';
import { Button, FormControlLabel, Switch, Typography } from '@material-ui/core';

type ESPHomeState = {
  switch_sw1: boolean,
  sensor_count: number,
  sensor_count_long: number,
};

//dev_node='test_esp8266.local'
function App() {
  const url = "http://test_esp8266.local";
  const [logs, setLog] = useState<string[]>([]);

  const [esphome, setEsphome] = useState<ESPHomeState>(
    {
      switch_sw1: false,
      sensor_count: 0,
      sensor_count_long: 0,
    });

  const [eventSource, eventSourceStatus] = useEventSource(url + "/events", false);
  useEventSourceListener(eventSource, ['log'], evt => {
    // setLog([
    //   ...logs,
    //   //...JSON.parse(evt.data)
    //   evt.data
    // ]);
    console.log('esphome: ', evt.data);
  }, [logs]);
  useEventSourceListener(eventSource, ['state'], evt => {
    const data = JSON.parse(evt.data);
    const id = (data.id as string).replace('-', '_');
    setEsphome({
      ...esphome,
      [id]: data.value
    });
  }, [esphome]);

  const setSwitch = (id: string, state: boolean) => {
    fetch(`${url}/switch/${id}/turn_${state ? "on" : "off"}`, { method: 'POST' });
  };

  return (
    <div>
      <div>Comunicación: {eventSourceStatus}</div>
      <Typography variant="h1">
        Cuenta Bolsas
      </Typography>
      <Typography variant="h2">
        Comun: {esphome.sensor_count}
      </Typography>

      <Typography variant="h2">
        Largas: {esphome.sensor_count_long}
      </Typography>
      {/*
      <FormControlLabel
        control={<Switch checked={esphome.switch_sw1} onChange={() => setSwitch("sw1", !esphome.switch_sw1)} />}
        label="Small"
      /> */}

      <Button variant="contained" size="large" color="primary" onClick={() => setSwitch("sw1", true)}>Reiniciar a cero</Button>

      {/* <div>
        {eventSourceStatus === "open" ? null : <div>Loading...</div>}
        {logs.map((msg) => <div>{msg}</div>)}
      </div> */}
    </div>
  );

}

export default App;
