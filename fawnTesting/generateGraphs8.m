files = dir('mpi8Times/*.txt');
%loop through all files in the directory
for i = 1:size(files)
    
    %load in information from the text file
    file = files(i);
    filename = strcat('mpi8Times/',file.name);
    fd = fopen(filename);
    file.name
    A = textscan(fd, '%d %d %d %d %f %f %f');
    numNodes = A{1};
    procPerNode = A{2};
    totalProc = A{3};
    rdvs = A{4};
    elapsedInfo = A{5};
    systemInfo = A{6};
    userInfo = A{7};
    
    %arrange information into readable form
    elapsedTime = zeros(8,8);
    elapsedTimeRdv = zeros(8,8);
    systemTime = zeros(8,8);
    systemTimeRdv = zeros(8,8);
    userTime = zeros(8,8);
    userTimeRdv = zeros(8,8);
    for j = 1:128
        if rdvs(j,1) == 0
            elapsedTime(numNodes(j,1),procPerNode(j,1)) = elapsedInfo(j,1);
        else 
            elapsedTimeRdv(numNodes(j,1),procPerNode(j,1)) = elapsedInfo(j,1);
        end 
        if rdvs(j,1) == 0
            systemTime(numNodes(j,1),procPerNode(j,1)) = double(systemInfo(j,1))/double(procPerNode(j,1));
        else 
            systemTimeRdv(numNodes(j,1),procPerNode(j,1)) = double(systemInfo(j,1))/double(procPerNode(j,1));
        end 
        if rdvs(j,1) == 0
            userTime(numNodes(j,1),procPerNode(j,1)) = double(userInfo(j,1))/double(procPerNode(j,1));
        else 
            userTimeRdv(numNodes(j,1),procPerNode(j,1)) = double(userInfo(j,1))/double(procPerNode(j,1));
        end
    end
    
    
    added = systemTime + userTime;
    addedRdv = systemTimeRdv + userTimeRdv;
    maxVal = ceil(max(added(:)));
    maxValRdv = ceil(max(addedRdv(:)));
    
    %plot 3D bar graph
    fig3 = figure;
    bar3(elapsedTime);
    title('Elapsed Time');
    ylabel('Number of Nodes');
    xlabel('Processes per node');
    zlabel('Time (s)');
    
    fig3Rdv = figure;
    bar3(elapsedTimeRdv);
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
    
    figE = figure;
    bar(1:8, [systemTime(:,5) userTime(:,5) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Five Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figF = figure;
    bar(1:8, [systemTime(:,6) userTime(:,6) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Six Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figG = figure;
    bar(1:8, [systemTime(:,7) userTime(:,7) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Seven Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figH = figure;
    bar(1:8, [systemTime(:,8) userTime(:,8) ], 0.5, 'stack');
    axis([0 9 0 maxVal]);
    title('Eight Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    
    savePath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_timedChart.jpeg');
    save3Path = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart.jpeg');
    saveAPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_1.jpeg');
    saveBPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_2.jpeg');
    saveCPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_3.jpeg');
    saveDPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_4.jpeg');
    saveEPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_5.jpeg');
    saveFPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_6.jpeg');
    saveGPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_7.jpeg');
    saveHPath = strcat('mpiTestGraphs8/NR/',file.name(1:end-4),'_elapsed3DChart_8.jpeg');
    print(fig, '-djpeg', savePath);
    print(figA, '-djpeg', saveAPath);
    print(figB, '-djpeg', saveBPath);
    print(figC, '-djpeg', saveCPath);
    print(figD, '-djpeg', saveDPath);
    print(figE, '-djpeg', saveEPath)
    print(figF, '-djpeg', saveFPath)
    print(figG, '-djpeg', saveGPath)
    print(figH, '-djpeg', saveHPath)
    print(fig3,'-djpeg',save3Path);
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %RDVS%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %plot the information
    figRdv = figure;
    subplot(4,1,1);
    bar(1:8, [systemTimeRdv(:,1) userTimeRdv(:,1) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('One Process per Node');
    subplot(4,1,2);
    bar(1:8, [systemTimeRdv(:,2) userTimeRdv(:,2) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Two Processes per Node');
    ylabel('Time (s)');
    subplot(4,1,3);
    bar(1:8, [systemTimeRdv(:,3) userTimeRdv(:,3) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Three Processes per Node');
    subplot(4,1,4);
    bar(1:8, [systemTimeRdv(:,4) userTimeRdv(:,4) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Four Processes per Node');
    xlabel('Number of Nodes');
    
    figARdv = figure;
    bar(1:8, [systemTimeRdv(:,1) userTimeRdv(:,1) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('One Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figBRdv = figure;
    bar(1:8, [systemTimeRdv(:,2) userTimeRdv(:,2) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Two Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figCRdv = figure;
    bar(1:8, [systemTimeRdv(:,3) userTimeRdv(:,3) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Three Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figDRdv = figure;
    bar(1:8, [systemTimeRdv(:,4) userTimeRdv(:,4) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Four Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figERdv = figure;
    bar(1:8, [systemTimeRdv(:,5) userTimeRdv(:,5) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Five Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figFRdv = figure;
    bar(1:8, [systemTimeRdv(:,6) userTimeRdv(:,6) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Six Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figGRdv = figure;
    bar(1:8, [systemTimeRdv(:,7) userTimeRdv(:,7) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Seven Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    figHRdv = figure;
    bar(1:8, [systemTimeRdv(:,8) userTimeRdv(:,8) ], 0.5, 'stack');
    axis([0 9 0 maxValRdv]);
    title('Eight Process per Node');
    xlabel('Number of Nodes');
    ylabel('Time (s)');
    
    
    savePathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_timedChart.jpeg');
    save3PathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart.jpeg');
    saveAPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_1.jpeg');
    saveBPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_2.jpeg');
    saveCPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_3.jpeg');
    saveDPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_4.jpeg');
    saveEPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_5.jpeg');
    saveFPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_6.jpeg');
    saveGPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_7.jpeg');
    saveHPathRdv = strcat('mpiTestGraphs8/RDVS/',file.name(1:end-4),'_elapsed3DChart_8.jpeg');
    print(figRdv, '-djpeg', savePathRdv);
    print(figARdv, '-djpeg', saveAPathRdv);
    print(figBRdv, '-djpeg', saveBPathRdv);
    print(figCRdv, '-djpeg', saveCPathRdv);
    print(figDRdv, '-djpeg', saveDPathRdv);
    print(figERdv, '-djpeg', saveEPathRdv);
    print(figFRdv, '-djpeg', saveFPathRdv);
    print(figGRdv, '-djpeg', saveGPathRdv);
    print(figHRdv, '-djpeg', saveHPathRdv);
    print(fig3Rdv,'-djpeg',save3PathRdv);
    
    close all;
    
    fclose(fd);
end