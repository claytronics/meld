files = dir('testInner2/*.txt');
%loop through all files in the directory
for i = 1:size(files)
    
    %load in information from the text file
    file = files(i);
    filename = strcat('testInner2/',file.name);
    fd = fopen(filename);
    file.name;
    A = textscan(fd, ' %d %d %d %d %d %f %f %f');
    numNodes = A{1};
    procPerNode = A{2};
    totalProc = A{3};
    rdvs = A{4};
    little = A{5};
    elapsedInfo = A{6};
    systemInfo = A{7};
    userInfo = A{8};
    
    elapsedInfoRdv = zeros(10,1);
    elapsedInfoNR = zeros(10,1);
    systemInfoRdv = zeros(10,1);
    systemInfoNR = zeros(10,1);
    userInfoRdv = zeros(10,1);
    userInfoNR = zeros(10,1);
    
    for j = 1:10;
                        elapsedInfoNR(j,1) = elapsedInfo(2*j-1,1);
                        systemInfoNR(j,1) = systemInfo(2*j-1,1)/double(procPerNode(2*j-1,1));
                        userInfoNR(j,1) = userInfo(2*j-1,1)/double(procPerNode(2*j-1,1));
                        elapsedInfoRdv(j,1) = elapsedInfo(2*j,1);
                        systemInfoRdv(j,1) = systemInfo(2*j,1)/double(procPerNode(2*j,1));
                        userInfoRdv(j,1) = userInfo(2*j,1)/double(procPerNode(2*j,1));
    end
            
    fig = figure;
    little2 = [1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000]';
    plot(little2,elapsedInfoRdv,'--rs');
    title(strcat('RDVS for ',file.name(1:end-4))); 
    xlabel('l');
    ylabel('Time (s)');
    
    
    figA = figure;
    plot(little2,elapsedInfoNR,'--rs');
    title(strcat('NR for ',file.name(1:end-4)));
    xlabel('l');
    ylabel('Time (s)');
    
    figB = figure;
    area(little2, [systemInfoRdv userInfoRdv]);
    title(strcat('RDVS for ',file.name(1:end-4))); 
    xlabel('l');
    ylabel('Time (s)');
    
    figC = figure;
    area(little2, [systemInfoNR userInfoNR]);
    xlabel('l');
    ylabel('Time (s)');
    title(strcat('NR for ',file.name(1:end-4))); 
    
    
    
    savePath = strcat('innerTestGraphs/RDVS/',file.name(1:end-4),'_elapsed.jpeg');
    savePathA = strcat('innerTestGraphs/NR/',file.name(1:end-4),'_elapsed.jpeg');
    savePathB = strcat('innerTestGraphs/RDVS/',file.name(1:end-4),'_sys-user.jpeg');
    savePathC = strcat('innerTestGraphs/NR/',file.name(1:end-4),'_sys-user.jpeg');
    print(fig, '-djpeg', savePath);
    print(figA, '-djpeg', savePathA);
    print(figB, '-djpeg', savePathB);
    print(figC, '-djpeg', savePathC);
    
    fclose(fd);
end