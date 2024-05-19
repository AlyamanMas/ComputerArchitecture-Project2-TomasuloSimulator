document.addEventListener('DOMContentLoaded', function() {
    fetch('/home/khalil/Desktop/courses/ComputerArch/Projii2/ComputerArchitecture-Project2-TomasuloSimulator/src/web/data.txt')
        .then(response => response.text())
        .then(data => {
            const lines = data.split('\n');
            let cycles = [];
            let currentCycle = {reservationData: [], cycleData: [], registerData: []};
            let currentSection = '';

            lines.forEach(line => {
                line = line.trim();
                if (line === '') return;

                if (line.startsWith('cycle:')) {
                    if (currentCycle.cycleData.length > 0 || currentCycle.reservationData.length > 0 || currentCycle.registerData.length > 0) {
                        cycles.push(currentCycle);
                        currentCycle = {reservationData: [], cycleData: [], registerData: []};
                    }
                    return;
                } else if (line === 'Reservation Stations:') {
                    currentSection = 'ReservationStations';
                    return;
                } else if (line === 'Instructions:') {
                    currentSection = 'Instructions';
                    return;
                } else if (line === 'Register Status Table:') {
                    currentSection = 'RegisterStatusTable';
                    return;
                }

                switch (currentSection) {
                    case 'ReservationStations':
                        const reservationParts = line.split(', ').map(part => part.split(': ').pop());
                        currentCycle.reservationData.push({
                            name: reservationParts[0],
                            busy: reservationParts[1] === '1' ? 'Yes' : 'No',
                            operation: reservationParts[2],
                            Vj: reservationParts[3],
                            Vk: reservationParts[4],
                            Qj: reservationParts[5],
                            Qk: reservationParts[6],
                            timeRemaining: reservationParts[7]
                        });
                        break;
                    case 'Instructions':
                        const instructionParts = line.split(', ').map(part => part.split(': ').pop());
                        currentCycle.cycleData.push({
                            instruction: `Instruction ${instructionParts[5]}`,
                            issue: instructionParts[0],
                            executeStart: instructionParts[1],
                            executeEnd: instructionParts[2],
                            writeBack: instructionParts[3]
                        });
                        break;
                    case 'RegisterStatusTable':
                        const registerParts = line.split(': ');
                        currentCycle.registerData.push({
                            register: registerParts[0],
                            status: registerParts[1],
                            reservationStation: 'N/A'
                        });
                        break;
                }
            });

            if (currentCycle.cycleData.length > 0 || currentCycle.reservationData.length > 0 || currentCycle.registerData.length > 0) {
                cycles.push(currentCycle);
            }

            const content = document.getElementById('content');
            cycles.forEach((cycle, index) => {
                const cycleContainer = document.createElement('div');
                cycleContainer.classList.add('cycle-container');

                const cycleTitle = document.createElement('h2');
                cycleTitle.textContent = `Cycle ${index}`;
                cycleContainer.appendChild(cycleTitle);

                cycleContainer.appendChild(createTable('Reservation Stations', 'reservationTable', cycle.reservationData));
                cycleContainer.appendChild(createTable('Instructions Cycle Data', 'cycleData', cycle.cycleData));
                cycleContainer.appendChild(createTable('Registers Status', 'registersTable', cycle.registerData));

                content.appendChild(cycleContainer);
            });

            function createTable(title, tableId, data) {
                const tableTitle = document.createElement('h2');
                tableTitle.textContent = title;

                const table = document.createElement('table');
                table.id = tableId;

                const thead = document.createElement('thead');
                const headerRow = document.createElement('tr');
                const headers = getHeaders(tableId);
                headers.forEach(header => {
                    const th = document.createElement('th');
                    th.textContent = header;
                    headerRow.appendChild(th);
                });
                thead.appendChild(headerRow);
                table.appendChild(thead);

                const tbody = document.createElement('tbody');
                data.forEach(rowData => {
                    const row = tbody.insertRow();
                    Object.values(rowData).forEach(text => {
                        const cell = row.insertCell();
                        cell.textContent = text;
                        if (tableId === 'reservationTable' && (text === 'Yes' || text === 'No')) {
                            cell.classList.add(text === 'Yes' ? 'highlight' : '');
                        }
                    });
                });
                table.appendChild(tbody);

                const container = document.createElement('div');
                container.appendChild(tableTitle);
                container.appendChild(table);

                return container;
            }

            function getHeaders(tableId) {
                switch (tableId) {
                    case 'reservationTable':
                        return ['Name', 'Busy', 'Operation', 'Vj', 'Vk', 'Qj', 'Qk', 'Time Remaining'];
                    case 'cycleData':
                        return ['Instruction', 'Issue', 'Execute Start', 'Execute End', 'Write Back'];
                    case 'registersTable':
                        return ['Register', 'Status', 'Reservation Station'];
                    default:
                        return [];
                }
            }
        })
        .catch(error => console.error('Error loading the data:', error));
});
