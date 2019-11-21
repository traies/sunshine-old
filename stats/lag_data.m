pkg load statistics;

lag_data = 4.167 .* fliplr(dlmread('lag_data.csv'));
boxplot(lag_data, 0, '.', 0);
axis([0 400 0 13]);
set(gca, "xtick", 0:25:400);
set(gca, "xgrid", "on");
set(gca (), "ytick", [12 11 10 9 8 7 6 5 4 3 2 1], "yticklabel", {"Nativo", "Nativo (vsync)", "Parsec", "Parsec (vsync)", "Moonlight", "Moonlight (vsync)", "Sunshine", "Sunshine (vsync)", "GamingAnywhere", "GamingAnywhere (vsync)", "Steam", "Steam (vsync)"});
xlabel('Tiempo de respuesta (ms)');