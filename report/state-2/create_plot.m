clear all; close all; clc;

%% Read data from CSV in the same folder
minimax = csvread('minimax.csv', 1, 0);
alphaBeta = csvread('alpha-beta.csv', 1, 0);
orderedAlphaBeta = csvread('ordered-alpha-beta.csv', 1, 0);

%% Generate plot
hold on;
f = fit(minimax(:,1), minimax(:,2), 'exp1');
plot(f, minimax(:,1), minimax(:,2));
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'));
grid on;
saveas(gcf, 'minimax.eps', 'epsc');

figure;
f = fit(alphaBeta(:,1), alphaBeta(:,2), 'exp1');
plot(f, alphaBeta(:,1), alphaBeta(:,2));
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'));
grid on;
saveas(gcf, 'alpha-beta.eps', 'epsc');

figure;
f = fit(orderedAlphaBeta(:,1), orderedAlphaBeta(:,2), 'exp1');
plot(f, orderedAlphaBeta(:,1), orderedAlphaBeta(:,2));
xlabel('Depth');
ylabel('States');
legend('data points', strcat(string(f.a), ' * (', string(exp(f.b)), ')^d'));
grid on;
saveas(gcf, 'ordered-alpha-beta.eps', 'epsc');
