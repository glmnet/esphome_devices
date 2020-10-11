import { Component } from "react";

import { useEffect, useRef, useState } from "react";

class ESPHome extends Component {

    // private const source: EventSource;

    // constructor(devUrl: string) {
    //     super();

    //     source = new EventSource("http://test_esp8266.local/events");

    //     this.source.addEventListener("x", {});
    // }


};

export default ESPHome;

// source.addEventListener('log', function (e) {
//     const log = document.getElementById("log");
//     let klass = '';
//     if (e.data.startsWith("[1;31m")) {
//         klass = 'e';
//     } else if (e.data.startsWith("[0;33m")) {
//         klass = 'w';
//     } else if (e.data.startsWith("[0;32m")) {
//         klass = 'i';
//     } else if (e.data.startsWith("[0;35m")) {
//         klass = 'c';
//     } else if (e.data.startsWith("[0;36m")) {
//         klass = 'd';
//     } else if (e.data.startsWith("[0;37m")) {
//         klass = 'v';
//     } else {
//         log.innerHTML += e.data + '\n';
//     }
//     log.innerHTML += '<span class="' + klass + '">' + e.data.substr(7, e.data.length - 10) + "</span>\n";
// });

// source.addEventListener('state', function (e) {
//     const data = JSON.parse(e.data);
//     document.getElementById(data.id).children[1].innerText = data.state;
// });

// const states = document.getElementById("states");
// let i = 0, row;
// for (; row = states.rows[i]; i++) {
//     if (row.classList.contains("switch")) {
//         (function (id) {
//             row.children[2].children[0].addEventListener('click', function () {
//                 const xhr = new XMLHttpRequest();
//                 xhr.open("POST", '/switch/' + id.substr(7) + '/toggle', true);
//                 xhr.send();
//             });
//         })(row.id);
//     }
//     if (row.classList.contains("fan")) {
//         (function (id) {
//             row.children[2].children[0].addEventListener('click', function () {
//                 const xhr = new XMLHttpRequest();
//                 xhr.open("POST", '/fan/' + id.substr(4) + '/toggle', true);
//                 xhr.send();
//             });
//         })(row.id);
//     }
//     if (row.classList.contains("light")) {
//         (function (id) {
//             row.children[2].children[0].addEventListener('click', function () {
//                 const xhr = new XMLHttpRequest();
//                 xhr.open("POST", '/light/' + id.substr(6) + '/toggle', true);
//                 xhr.send();
//             });
//         })(row.id);
//     }
//     if (row.classList.contains("cover")) {
//         (function (id) {
//             row.children[2].children[0].addEventListener('click', function () {
//                 const xhr = new XMLHttpRequest();
//                 xhr.open("POST", '/cover/' + id.substr(6) + '/open', true);
//                 xhr.send();
//             });
//             row.children[2].children[1].addEventListener('click', function () {
//                 const xhr = new XMLHttpRequest();
//                 xhr.open("POST", '/cover/' + id.substr(6) + '/close', true);
//                 xhr.send();
//             });
//         })(row.id);
//     }
// }


type EventSourceConstructor = { new(url: string, eventSourceInitDict?: EventSourceInit): EventSource };

export type EventSourceStatus = "init" | "open" | "closed" | "error";

export type EventSourceEvent = Event & { data: string };

export function useEventSource(url: string, withCredentials?: boolean, ESClass: EventSourceConstructor = EventSource) {
    const source = useRef<EventSource | null>(null);
    const [status, setStatus] = useState<EventSourceStatus>("init");
    useEffect(() => {
        if (url) {
            const es = new ESClass(url, { withCredentials });
            source.current = es;

            es.addEventListener("open", () => setStatus("open"));
            es.addEventListener("error", () => setStatus("error"));

            return () => {
                source.current = null;
                es.close();
            };
        }

        setStatus("closed");
    }, [url, withCredentials, ESClass]);

    return [source.current, status] as const;
}

export function useEventSourceListener(
    source: EventSource | null,
    types: string[],
    listener: (e: EventSourceEvent) => void,
    dependencies: any[] = []
) {
    useEffect(() => {
        if (source) {
            types.forEach((type) => source.addEventListener(type, listener as any));
            return () => types.forEach((type) => source.removeEventListener(type, listener as any));
        }
    }, [source, ...dependencies]);
}