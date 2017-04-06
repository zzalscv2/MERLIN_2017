
%% Import LatticeFunctionTable

filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/FCC_v7/5_APR_Transport/LatticeFunctions/LatticeFunctions.dat';
formatSpec = '%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%f%[^\n\r]';
fileID = fopen(filename,'r');
dataArray = textscan(fileID, formatSpec, 'Delimiter', '', 'WhiteSpace', '', 'EmptyValue' ,NaN, 'ReturnOnError', false);
fclose(fileID);

% Allocate imported array to column variable names
M_s = dataArray{:, 1};
M_x = dataArray{:, 2};
% xp = dataArray{:, 3};
M_y = dataArray{:, 4};
% yp = dataArray{:, 5};
% mu_x_frac = dataArray{:, 6};
% mu_y_frac = dataArray{:, 7};
M_betax = dataArray{:, 8};
% Alpha_x = dataArray{:, 9};
M_betay = dataArray{:, 10};
% Alpha_y = dataArray{:, 11};
% D_x_EF = dataArray{:, 12};      % D_x * Energy scaling factor
% D_xp_EF = dataArray{:, 13};      
% D_y_EF = dataArray{:, 14};      % D_y * Energy scaling factor
% D_yp_EF = dataArray{:, 15};
% EF = dataArray{:, 16};          % Energy scaling factor
% Dx_lf = dataArray{:, 12} / dataArray{:, 16};
% Dy_lf = dataArray{:, 14} / dataArray{:, 16};
% Perform operations

% Dx_lf = D_x_EF / EF;
% Dy_lf = D_y_EF / EF;
% D_xp = D_xp_EF / EF;
% D_yp = D_yp_EF / EF;

% Alpha_x = -1 .* Alpha_x;
% Alpha_y = -1 .* Alpha_y;

clearvars filename formatSpec fileID dataArray ans;

%% Import Dispersion

% filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/LatticeTest/26JulyTest/LatticeFunctions/Dispersion.dat';
filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/FCC_v7/5_APR_Transport/LatticeFunctions/Dispersion.dat';
formatSpec = '%14f%14f%f%[^\n\r]';
fileID = fopen(filename,'r');
dataArray = textscan(fileID, formatSpec, 'Delimiter', '', 'WhiteSpace', '', 'EmptyValue' ,NaN, 'ReturnOnError', false);
fclose(fileID);
M_S2 = dataArray{:, 1};
M_Dx = dataArray{:, 2};
M_Dy = dataArray{:, 3};
clearvars filename formatSpec fileID dataArray ans;

%% Import LatticeFunctionTable

% filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/LatticeTest/26JulyTest/LatticeFunctions/LatticeFunctions.dat';
filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/FCC_v7/5_APR_Symplectic/LatticeFunctions/LatticeFunctions.dat';
formatSpec = '%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%30f%f%[^\n\r]';
fileID = fopen(filename,'r');
dataArray = textscan(fileID, formatSpec, 'Delimiter', '', 'WhiteSpace', '', 'EmptyValue' ,NaN, 'ReturnOnError', false);
fclose(fileID);

% Allocate imported array to column variable names
s = dataArray{:, 1};
x = dataArray{:, 2};
% xp = dataArray{:, 3};
y = dataArray{:, 4};
% yp = dataArray{:, 5};
% mu_x_frac = dataArray{:, 6};
% mu_y_frac = dataArray{:, 7};
betax = dataArray{:, 8};
% Alpha_x = dataArray{:, 9};
betay = dataArray{:, 10};
% Alpha_y = dataArray{:, 11};
% D_x_EF = dataArray{:, 12};      % D_x * Energy scaling factor
% D_xp_EF = dataArray{:, 13};      
% D_y_EF = dataArray{:, 14};      % D_y * Energy scaling factor
% D_yp_EF = dataArray{:, 15};
% EF = dataArray{:, 16};          % Energy scaling factor
% Dx_lf = dataArray{:, 12} / dataArray{:, 16};
% Dy_lf = dataArray{:, 14} / dataArray{:, 16};
% Perform operations

% Dx_lf = D_x_EF / EF;
% Dy_lf = D_y_EF / EF;
% D_xp = D_xp_EF / EF;
% D_yp = D_yp_EF / EF;

% Alpha_x = -1 .* Alpha_x;
% Alpha_y = -1 .* Alpha_y;

clearvars filename formatSpec fileID dataArray ans;

%% Import Dispersion

% filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/LatticeTest/26JulyTest/LatticeFunctions/Dispersion.dat';
filename = '/home/HR/Downloads/MERLIN_HRThesis/MERLIN/Build/FCC/outputs/FCC_v7/5_APR_Symplectic/LatticeFunctions/Dispersion.dat';
formatSpec = '%14f%14f%f%[^\n\r]';
fileID = fopen(filename,'r');
dataArray = textscan(fileID, formatSpec, 'Delimiter', '', 'WhiteSpace', '', 'EmptyValue' ,NaN, 'ReturnOnError', false);
fclose(fileID);
S_D = dataArray{:, 1};
Dx = dataArray{:, 2};
Dy = dataArray{:, 3};
clearvars filename formatSpec fileID dataArray ans;

%% Multiple Plots

% xlimits full
xmin = 0;
xmax = 97387.4336310000;

% xlimits zoom start
% xmin = 0;
% xmax = 100;

% IPA - IPB
% xmin = 0;
% xmax = 7000;

%% Plot beta x
figure;
subplot(2,1,1);

plot(s, betax, '-', M_s, M_betax, ':','Linewidth',1.5);

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
legend('Transport','Symplectic');
ylabel('\beta_x [m]');
xlabel('s [m]');
grid on;

% Plot beta x difference
% figure;
subplot(2,1,2);

% interpolation steps
interval = 1E-3;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(s, betax);
array_inm = horzcat(M_s, M_betax);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta\beta_x [m]');
xlabel('s [m]');
grid on;

%% Plot beta y
figure;
subplot(2,1,1);

plot(s, betay, '-', M_s, M_betay, ':','Linewidth',1.5);

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
legend('Transport','Symplectic');
ylabel('\beta_y [m]');
xlabel('s [m]');
grid on;

% Plot beta y difference
% figure;
subplot(2,1,2);

% interpolation steps
interval = 1;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(s, betay);
array_inm = horzcat(M_s, M_betay);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta\beta_y [m]');
xlabel('s [m]');
grid on;

%% Plot Dx
figure;
subplot(2,1,1);

% plot(S_D, Dx, '-', M_s, M_Dx, ':', s, Dx_lf, '--','Linewidth',1.5);
plot(S_D, Dx, '-', M_S2, M_Dx, ':','Linewidth',1.5);
% plot(s, Dx_lf, '--', S_D, Dx, '-', M_s, M_Dx, ':','Linewidth',1.5);
% plot(s, Dx_lf, '-', S_D, Dx,'Linewidth',1.5);

set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
% legend('MERLIN Dispersion','MADX','MERLIN LatticeFunctions');
legend('Transport','Symplectic');
ylabel('D_x [m]');
xlabel('s [m]');
grid on;

% Plot Dx difference
subplot(2,1,2);

% interpolation steps
interval = 10;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(S_D, Dx);
% array_in = horzcat(s, Dx_lf);
array_inm = horzcat(M_S2, M_Dx);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta D_x [m]');
xlabel('s [m]');
grid on;

%% Plot Dy
figure;
subplot(2,1,1);

% plot(S_D, Dy, '-', M_s, M_Dy, ':', s, Dy_lf, '--','Linewidth',1.5);
plot(S_D, Dy, '-', M_S2, M_Dy, ':','Linewidth',1.5);

set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
% legend('MERLIN Dispersion','MADX','MERLIN LatticeFunctions');
legend('Transport','Symplectic');
ylabel('D_y [m]');
xlabel('s [m]');
grid on;

% Plot Dx difference
subplot(2,1,2);

% interpolation steps
interval = 1;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(S_D, Dy);
% array_in = horzcat(s, Dy_lf);
array_inm = horzcat(M_S2, M_Dy);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta D_y [m]');
xlabel('s [m]');
grid on;

%% Plot x
figure;
subplot(2,1,1);

plot(s, x, '-', M_s, M_x, ':','Linewidth',1.5);

% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
legend('Transport','Symplectic');
ylabel('x [m]');
xlabel('s [m]');
grid on;

% Plot x difference
subplot(2,1,2);

% interpolation steps
interval = 0.01;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(s, x);
array_inm = horzcat(M_s, M_x);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta x [m]');
xlabel('s [m]');
grid on;

%% Plot y
figure;
subplot(2,1,1);

plot(s, y, '-', M_s, M_y, ':','Linewidth',1.5);

% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
title('FCC-hh v7 Beam 1');
legend('Transport','Symplectic');
ylabel('y [m]');
xlabel('s [m]');
grid on;

hold off;

% Plot y difference
subplot(2,1,2);

% interpolation steps
interval = 0.01;
s_int = 0:interval:xmax;

% create 2D array of data
array_in = horzcat(s, y);
array_inm = horzcat(M_s, M_y);

% indices of unique s positions
[~, ind] = unique(array_in(:,1), 'rows', 'first');
[~, indm] = unique(array_inm(:,1), 'rows', 'first');

% arrays of unique s points
test1 = array_in(ind,:);
test2 = array_inm(indm,:);

% interpolate MERLIN and MADX
merlin_int = interp1(test1(:,1), test1(:,2), s_int, 'linear','extrap');
mad_int = interp1(test2(:,1), test2(:,2), s_int, 'linear','extrap');

plot(s_int, (merlin_int - mad_int) );

% set(gca,'yscale','log','FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'FontSize',16,'PlotBoxAspectratio',[4 2 2],'Linewidth',1,'XLim',[xmin xmax]);
% set(gca,'yscale','log','FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
set(gca,'FontSize',16,'Linewidth',1,'XLim',[xmin xmax]);
% title('6.5 TeV Beam 1');
legend('Transport','Symplectic');
ylabel('\Delta y [m]');
xlabel('s [m]');
grid on;