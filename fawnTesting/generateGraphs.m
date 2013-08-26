files = dir('mpiNR/*.txt');
%loop through all files in the directory
for i = 1:size(files)
    
    %load in information from the text file
    file = files(i);
    filename = strcat('mpiNR/',file.name);
    fd = fopen(filename);
    file.name
    A = textscan(fd, ' %d %d %d %d %f %f %f');
    numNodes = A{1};
    procPerNode = A{2};
    totalProc = A{3};
    rdvs = A{4};
    elapsedInfo = A{5};
    systemInfo = A{6};
    userInfo = A{7};
    
    
    %arrange information into readable form
    elapsedTime = zeros(8,4);
    systemTime = zeros(8,4);
    userTime = zeros(8,4);
    for j = 1:32
        elapsedTime(numNodes(j,1),procPerNode(j,1)) = elapsedInfo(j,1);
        systemTime(numNodes(j,1),procPerNode(j,1)) = double(systemInfo(j,1))/double(procPerNode(j,1));
        userTime(numNodes(j,1),procPerNode(j,1)) = double(userInfo(j,1))/double(procPerNode(j,1));
    end
    
    
    added = systemTime + userTime;
    maxVal = ceil(max(added(:)));
    
    %plot 3D bar graph
    fig3 = figure;
    bar3(elapsedTime);
    title('Elapsed Time');
    ylabel('Number of Nodes');
    xlabel('Processes per node');
    zlabel('Time (s)');
    
    
    %plot the information
    fig = figure;
    subplot(4,1,1);
    bar(1:8, [systemTime(:,1) userTime(:,1) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('One Process per Node');
    subplot(4,1,2);
    bar(1:8, [systemTime(:,2) userTime(:,2) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Two Processes per Node');
    ylabel('Time (s)');
    subplot(4,1,3);
    bar(1:8, [systemTime(:,3) userTime(:,3) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Three Processes per Node');
    subplot(4,1,4);
    bar(1:8, [systemTime(:,4) userTime(:,4) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Four Processes per Node');
    xlabel('Number of Nodes');
    
    figA = figure;
    bar(1:8, [systemTime(:,1) userTime(:,1) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('One Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figB = figure;
    bar(1:8, [systemTime(:,2) userTime(:,2) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Two Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figC = figure;
    bar(1:8, [systemTime(:,3) userTime(:,3) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Three Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figD = figure;
    bar(1:8, [systemTime(:,4) userTime(:,4) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Four Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    
    savePath = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_timedChart.jpeg');
    save3Path = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_elapsed3DChart.jpeg');
    saveAPath = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_elapsed3DChart_1.jpeg');
    saveBPath = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_elapsed3DChart_2.jpeg');
    saveCPath = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_elapsed3DChart_3.jpeg');
    saveDPath = strcat('mpiTestGraphs/NR2/',file.name(1:end-4),'_elapsed3DChart_4.jpeg');
    print(fig, '-djpeg', savePath);
    print(figA, '-djpeg', saveAPath);
    print(figB, '-djpeg', saveBPath);
    print(figC, '-djpeg', saveCPath);
    print(figD, '-djpeg', saveDPath);
    print(fig3,'-djpeg',save3Path);
    
    close all;
    
    fclose(fd);
end