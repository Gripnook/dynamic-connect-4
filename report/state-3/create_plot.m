clear all; close all; clc;

%% Read data from CSV in the same folder
minimax = csvread('minimax.csv', 1, 0);
alphaBeta = csvread('alpha-beta.csv', 1, 0);
orderedAlphaBeta = csvread('ordered-alpha-beta.csv', 1, 0);

%% Generate plot
hold on;
f = fit(minimax(:,1), minimax(:,2), 'exp1');
plot(f, minimax(:,1), minimax(:,2));
dx = range(minimax(:,1))./8;
dy = range(minimax(:,2))./16;
text(minimax(:,1), minimax(:,2) + dy, cellstr(num2str(minimax(:,2))), 'HorizontalAlignment', 'right');
xlim([min(minimax(:,1))-dx, max(minimax(:,1))+dx]);
ylim([min(minimax(:,2))-dy, max(minimax(:,2))+2*dy]);
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'), 'Location', 'northwest');
grid on;
saveas(gcf, 'minimax.eps', 'epsc');

figure;
f = fit(alphaBeta(:,1), alphaBeta(:,2), 'exp1');
plot(f, alphaBeta(:,1), alphaBeta(:,2));
dx = range(alphaBeta(:,1))./8;
dy = range(alphaBeta(:,2))./16;
text(alphaBeta(:,1), alphaBeta(:,2) + dy, cellstr(num2str(alphaBeta(:,2))), 'HorizontalAlignment', 'right');
xlim([min(alphaBeta(:,1))-dx, max(alphaBeta(:,1))+dx]);
ylim([min(alphaBeta(:,2))-dy, max(alphaBeta(:,2))+2*dy]);
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'), 'Location', 'northwest');
grid on;
saveas(gcf, 'alpha-beta.eps', 'epsc');

figure;
f = fit(orderedAlphaBeta(:,1), orderedAlphaBeta(:,2), 'exp1');
plot(f, orderedAlphaBeta(:,1), orderedAlphaBeta(:,2));
dx = range(orderedAlphaBeta(:,1))./8;
dy = range(orderedAlphaBeta(:,2))./16;
text(orderedAlphaBeta(:,1), orderedAlphaBeta(:,2) + dy, cellstr(num2str(orderedAlphaBeta(:,2))), 'HorizontalAlignment', 'right');
xlim([min(orderedAlphaBeta(:,1))-dx, max(orderedAlphaBeta(:,1))+dx]);
ylim([min(orderedAlphaBeta(:,2))-dy, max(orderedAlphaBeta(:,2))+2*dy]);
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'), 'Location', 'northwest');
grid on;
saveas(gcf, 'ordered-alpha-beta.eps', 'epsc');
