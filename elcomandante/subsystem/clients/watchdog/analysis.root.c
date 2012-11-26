{
	gROOT->Reset();
	const int MAX_RESULTS = 10000;
	struct {
		time_t	first_time;
		int	n;		// number of entries in arrays
		int	xi[MAX_RESULTS];
		double	xf[MAX_RESULTS];
		time_t	timestamp[MAX_RESULTS];
		char	valid[10][MAX_RESULTS];
		char	flags[10][MAX_RESULTS];
		double	lastint[MAX_RESULTS];
		double	deltaT[MAX_RESULTS];
		double	deltaT_sigma[MAX_RESULTS];
		double	rate[MAX_RESULTS];
		double	rate_sigma[MAX_RESULTS];
		int	N[MAX_RESULTS];
		int	LateCount[MAX_RESULTS];
		double	LatePercent[MAX_RESULTS];
		int	EarlyCount[MAX_RESULTS];
		double	EarlyPercent[MAX_RESULTS];
	} results;
	results.n = 0;
	results.first_time = 0;

	// defined fields
	typedef struct {
		time_t	timestamp;
		char	valid[10];
		char	flags[10];
		double	lastint;
		double	deltaT;
		double	deltaT_sigma;
		double	rate;
		double	rate_sigma;
		int	N;
		int	LateCount;
		double	LatePercent;
		int	EarlyCount;
		double	EarlyPercent;
	} event_t;
	
	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////  R E A D   F I L E  ////////
	/////////////////////////////////////////////////////////////////////////////////
	
	const char* DATAFILE="quickdata_4Sigma";
	printf("Opening data File %s\n", DATAFILE); fflush(stdout);
	ifstream* f = new ifstream(DATAFILE);
	if (f->good()) cout << "good" << endl;

	// 1237806049: valid ----, lastint=1.001851, deltaT=(0.993111+-0.037920)sec == rate=(1.006936+-0.038448)Hz, N=27, Late=0, 0.000000%, Early=0, 0.000000%
	event_t linedata;
	int ret;
	char line[500];

	while (f->good() && results.n < MAX_RESULTS) {
		f->getline(line, 500);
	//	bzero(&linedata, sizeof(linedata));
		//*f >> interv >> late[n] >> lateperc[n] >> early[n] >> earlyperc[n];
		if ( ( ret=sscanf(line, "%1$ld: %2$s %3$s lastint=%4$lf, deltaT=(%5$lf+-%6$lf)sec == rate=(%7$lf+-%9$lf)Hz, N=%*d, Late=%9$d, %*lf%%, Early=%10$d, %*lf%%", 
			&(linedata.timestamp),
			&(linedata.valid),
			&(linedata.flags),
			&(linedata.lastint),
			&(linedata.deltaT),
			&(linedata.deltaT_sigma),
			&(linedata.rate),
			&(linedata.rate_sigma),
			&(linedata.LateCount),
			&(linedata.EarlyCount)
			)) != 10
		     ) {
			linedata.valid[9]=0;
			linedata.flags[9]=0;
			printf("line %d invalid (ret=%d): \n", results.n, ret);
			if ( ret == -1 ) break;
			printf("%s\n", line);
			printf("%ld: %s %s lastint=%lf, deltaT=(%lf+-%lf)sec == rate=(%lf+-%lf)Hz, Late=%d, Early=%d\n", 
				(linedata.timestamp),
				(linedata.valid),
				(linedata.flags),
				(linedata.lastint),
				(linedata.deltaT),
				(linedata.deltaT_sigma),
				(linedata.rate),
				(linedata.rate_sigma),
				(linedata.LateCount),
				(linedata.EarlyCount));
			printf("\n"); fflush(stdout);
			return 1;
		} else {
			// appednd linedata to result arrays
			results.xi[results.n] = results.n;
			results.xf[results.n] = (float)results.n;
			results.timestamp[results.n] = linedata.timestamp;
			strncpy(&(results.valid[0][results.n]), linedata.valid, 10);
			strncpy(&(results.flags[0][results.n]), linedata.flags, 10);
			results.lastint[results.n] = linedata.lastint;
			results.deltaT[results.n] = linedata.deltaT;
			results.deltaT_sigma[results.n] = linedata.deltaT_sigma;
			results.rate[results.n] = linedata.rate;
			results.rate_sigma[results.n] = linedata.rate_sigma;
			results.N[results.n] = linedata.N;
			results.LateCount[results.n] = linedata.LateCount;
			results.LatePercent[results.n] = linedata.LatePercent;
			results.EarlyCount[results.n] = linedata.EarlyCount;
			results.EarlyPercent[results.n] = linedata.EarlyPercent;
			//printf("filling lastint=%lf\n", linedata.lastint); fflush(stdout);
			if ( results.n==0 ) { results.first_time = linedata.timestamp; }
			results.n++;
		}
	}
	//f.close();
	printf("File read successfully (%d lines).\n", results.n); fflush(stdout);
	
	if (results.n==0) {
		printf("no results. exit.\n"); fflush(stdout);
		return 0;
	}

	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////  A N A L Y S I S ///////////
	/////////////////////////////////////////////////////////////////////////////////

	printf("Creating Graphs:\n"); fflush(stdout);
	
	printf("\tTH1F intInst\n"); fflush(stdout);
		TH1F* intHist = new TH1F("intInst", "Interval", 200, 0.1017, 0.1021);
		for (int i=0; i<results.n; i++) {
			intHist->Fill(results.lastint[i]);
		}
	
	printf("\tTGraph lastInt\n"); fflush(stdout);
		TGraph* lastInt = new TGraph(results.n, results.xf, results.lastint);
		lastInt->SetName("lastInt");
		lastInt->SetTitle("Last Interval");
		lastInt->SetLineColor(1);
		lastInt->SetLineWidth(1);

	printf("\tTGraph deltaT\n"); fflush(stdout);
		TGraph* deltaT = new TGraph(results.n, results.xf, results.deltaT);
		deltaT->SetName("deltaT");
		deltaT->SetTitle("Mean deltaT");
		deltaT->SetLineColor(2);
		deltaT->SetLineWidth(1);

	printf("\tTGraph deltaT_sigma\n"); fflush(stdout);
		TGraph* deltaT_sigma = new TGraph(results.n, results.xf, results.deltaT_sigma);
		deltaT_sigma->SetName("deltaT_sigma");
		deltaT_sigma->SetTitle("deltaT deviation");
		deltaT_sigma->SetLineColor(3);
		deltaT_sigma->SetLineWidth(1);

	printf("\tTGraph lateGraph\n"); fflush(stdout);
		TGraph* lateGraph = new TGraph(results.n, results.xi, results.LateCount);
		lateGraph->SetName("lateGraph");
		lateGraph->SetTitle("Cummulative Late Count");
		lateGraph->SetLineColor(4);
		lateGraph->SetLineWidth(1);

	printf("\tTGraph earlyGraph\n"); fflush(stdout);
		TGraph* earlyGraph = new TGraph(results.n, results.xi, results.LateCount);
		earlyGraph->SetName("earlyGraph");
		earlyGraph->SetTitle("Cummulative Early Count");
		earlyGraph->SetLineColor(5);
		earlyGraph->SetLineWidth(1);

	printf("Graphs done.\n"); fflush(stdout);

	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////  S A V E  //////////
	/////////////////////////////////////////////////////////////////////////////////

	const char* ROOTFILE="Analysis.root";
	printf("writing Root file\n"); fflush(stdout);
	TFile rootfile(ROOTFILE, "recreate");
//	rootfile.cd();
	intHist->Write("intHist");
	lateGraph->Write("lateGraph");
	deltaT_sigma->Write("deltaT_sigma");
	deltaT->Write("deltaT");
	lastInt->Write("lastInt");
	printf("closing Root file.\n"); fflush(stdout);
	rootfile.Close();

	/////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////  D I S P L A Y   ///////////
	/////////////////////////////////////////////////////////////////////////////////

	printf("Drawing to Canvas\n"); fflush(stdout);

	TCanvas c1("mycanvas", "Results");
	c1.Divide(3,2);

	c1.cd(1); intHist->Draw();
	c1.cd(2); lastInt->Draw("APL");
	c1.cd(3); deltaT->Draw("APL");
	c1.cd(4); deltaT_sigma->Draw("APL");
	c1.cd(5); lateGraph->Draw("APL");
	c1.cd(6); earlyGraph->Draw("APL");

	gStyle->SetOptFit(111);
	lateGraph->Fit("pol1");
	earlyGraph->Fit("pol1");
	
	printf("bye.\n"); fflush(stdout);
}
