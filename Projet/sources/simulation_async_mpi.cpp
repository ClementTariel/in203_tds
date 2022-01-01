#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
#include "contexte.hpp"
#include "individu.hpp"
#include "graphisme/src/SDL2/sdl2.hpp"

#include <chrono>
# include <mpi.h>




void màjStatistique( épidémie::Grille& grille, std::vector<épidémie::Individu> const& individus )
{
    for ( auto& statistique : grille.getStatistiques() )
    {
        statistique.nombre_contaminant_grippé_et_contaminé_par_agent = 0;
        statistique.nombre_contaminant_seulement_contaminé_par_agent = 0;
        statistique.nombre_contaminant_seulement_grippé              = 0;
    }
    auto [largeur,hauteur] = grille.dimension();
    auto& statistiques = grille.getStatistiques();
    for ( auto const& personne : individus )
    {
        auto pos = personne.position();

        std::size_t index = pos.x + pos.y * largeur;
        if (personne.aGrippeContagieuse() )
        {
            if (personne.aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_grippé_et_contaminé_par_agent += 1;
            }
            else 
            {
                statistiques[index].nombre_contaminant_seulement_grippé += 1;
            }
        }
        else
        {
            if (personne.aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_seulement_contaminé_par_agent += 1;
            }
        }
    }
}

void afficheSimulation(sdl2::window& écran, épidémie::Grille const& grille, std::size_t jour)
{
    auto [largeur_écran,hauteur_écran] = écran.dimensions();
    auto [largeur_grille,hauteur_grille] = grille.dimension();
    auto const& statistiques = grille.getStatistiques();
    sdl2::font fonte_texte("./graphisme/src/data/Lato-Thin.ttf", 18);
    écran.cls({0x00,0x00,0x00});
    // Affichage de la grille :
    std::uint16_t stepX = largeur_écran/largeur_grille;
    unsigned short stepY = (hauteur_écran-50)/hauteur_grille;
    double factor = 255./15.;

    for ( unsigned short i = 0; i < largeur_grille; ++i )
    {
        for (unsigned short j = 0; j < hauteur_grille; ++j )
        {
            auto const& stat = statistiques[i+j*largeur_grille];
            int valueGrippe = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_grippé;
            int valueAgent  = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_contaminé_par_agent;
            std::uint16_t origx = i*stepX;
            std::uint16_t origy = j*stepY;
            std::uint8_t red = valueGrippe > 0 ? 127+std::uint8_t(std::min(128., 0.5*factor*valueGrippe)) : 0;
            std::uint8_t green = std::uint8_t(std::min(255., factor*valueAgent));
            std::uint8_t blue= std::uint8_t(std::min(255., factor*valueAgent ));
            écran << sdl2::rectangle({origx,origy}, {stepX,stepY}, {red, green,blue}, true);
        }
    }

    écran << sdl2::texte("Carte population grippée", fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(largeur_écran/2, hauteur_écran-20);
    écran << sdl2::texte(std::string("Jour : ") + std::to_string(jour), fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(0,hauteur_écran-20);
    écran << sdl2::flush;
}

void simulation(bool affiche, int rank,int nbp)
{
    MPI_Comm scomm;
    int color = rank == 0;
    MPI_Comm_split( MPI_COMM_WORLD, color, rank, &scomm );

    MPI_Status status ;

    MPI_Status quit_status ;

    MPI_Request quit_request ;

    int ready_to_receive_flag = 0;

    bool ready_to_receive;

    //MPI_Request jours_request ;

    //MPI_Request grille_request ;    


    int nb_iteration = 0;

    constexpr const unsigned int largeur_écran = 1280, hauteur_écran = 1024;
    
    sdl2::window écran("Simulation épidémie de grippe", {largeur_écran,hauteur_écran});
    if (rank != 0){
        sdl2::finalize();
    }


    unsigned int graine_aléatoire = 1;
    std::uniform_real_distribution<double> porteur_pathogène(0.,1.);


    épidémie::ContexteGlobal contexte;
    // contexte.déplacement_maximal = 1; <= Si on veut moins de brassage
    contexte.taux_population = 3*100'020;
    //contexte.taux_population = 1'000;
    contexte.interactions.β = 60.;
    std::vector<épidémie::Individu> population;
    population.reserve(contexte.taux_population);
    épidémie::Grille grille{contexte.taux_population};

    auto [largeur_grille,hauteur_grille] = grille.dimension();
    // L'agent pathogène n'évolue pas et reste donc constant...
    épidémie::AgentPathogène agent(graine_aléatoire++);
    // Initialisation de la population initiale :
    for (std::size_t i = 0; i < contexte.taux_population; ++i )
    {
        std::default_random_engine motor(100*(i+1));
        population.emplace_back(graine_aléatoire++, contexte.espérance_de_vie, contexte.déplacement_maximal);
        population.back().setPosition(largeur_grille, hauteur_grille);
        if (porteur_pathogène(motor) < 0.2)
        {
            population.back().estContaminé(agent);   
        }
    }

    std::size_t jours_écoulés = 0;
    int         jour_apparition_grippe = 0;
    int         nombre_immunisés_grippe= (contexte.taux_population*23)/100;
    sdl2::event_queue queue;

    bool quitting = false;

    std::ofstream output("Courbe.dat");
    output << "# jours_écoulés \t nombreTotalContaminésGrippe \t nombreTotalContaminésAgentPathogène()" << std::endl;

    épidémie::Grippe grippe(0);


    std::cout << "Début boucle épidémie" << std::endl << std::flush;

    std::chrono::time_point<std::chrono::system_clock> start, middle, end;

    bool zero_quit = false;
    int flag;
    if (rank >0){
        MPI_Irecv(&zero_quit,1,MPI_C_BOOL,0,1,MPI_COMM_WORLD,& quit_request );
        //std::cout << "rank "<<rank<<" quitting = false" << std::endl;
            
    }

    //temp
    std::vector<épidémie::Grille::StatistiqueParCase> temp_grid;
    temp_grid.reserve(contexte.taux_population);
    temp_grid = grille.getStatistiques();

    //MPI_Barrier(MPI_COMM_WORLD);
    
    while (!quitting)
    {
        start = std::chrono::system_clock::now();
        nb_iteration ++;


        if (rank == 0){
            auto events = queue.pull_events();
            for ( const auto& e : events)
            {
                if (e->kind_of_event() == sdl2::event::quit){
                    quitting = true;
                    zero_quit = true;
                    for (int k=1;k<nbp;k++){
                        MPI_Isend(&zero_quit, 1, MPI_C_BOOL, k, 1, MPI_COMM_WORLD,& quit_request ) ;
                        //std::cout << "rank 0 to rank "<<k<<" send quitting message" << std::endl;
            
                    }
                }
            }
            //reception des donnees
            //std::cout << "rank "<<rank<<" trying to receive message" << std::endl;
            //MPI_Irecv(&jours_écoulés,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,& jours_request );
            if (!quitting){
                MPI_Send(&ready_to_receive, 1, MPI_INT, 1, 4, MPI_COMM_WORLD);
                
                MPI_Recv(&jours_écoulés , 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD ,& status );
                
                //MPI_Irecv(temp_grid.data(), 3*largeur_grille*hauteur_grille, MPI_INT, MPI_ANY_SOURCE,0,MPI_COMM_WORLD,& grille_request );
                MPI_Recv(temp_grid.data(), 3*largeur_grille*hauteur_grille, MPI_INT, MPI_ANY_SOURCE, 3, MPI_COMM_WORLD ,& status );
                //std::cout << "1 : "<< sizeof(temp_grid) <<","<< sizeof(grille.getStatistiques()) << std::endl;
                grille.set_m_statistiques(temp_grid);
                //std::cout << "2 : "<< sizeof(temp_grid) <<","<< sizeof(grille.getStatistiques()) << std::endl;
                afficheSimulation(écran, grille, jours_écoulés);
                //std::cout << "rank "<<rank<<" message received" << std::endl;
            }
            
        }
        if(rank > 0){

            //if (rank == 1){
                if (jours_écoulés%365 == 0)// Si le premier Octobre (début de l'année pour l'épidémie ;-) )
                {
                    grippe = épidémie::Grippe(jours_écoulés/365);
                    jour_apparition_grippe = grippe.dateCalculImportationGrippe();
                    grippe.calculNouveauTauxTransmission();
                    // 23% des gens sont immunisés. On prend les 23% premiers
                    for ( int ipersonne = 0; ipersonne < nombre_immunisés_grippe; ++ipersonne)
                    {
                        population[ipersonne].devientImmuniséGrippe();
                    }
                    for ( int ipersonne = nombre_immunisés_grippe; ipersonne < int(contexte.taux_population); ++ipersonne )
                    {
                        population[ipersonne].redevientSensibleGrippe();
                    }
                }
                if (jours_écoulés%365 == std::size_t(jour_apparition_grippe))
                {
                    for (int ipersonne = nombre_immunisés_grippe; ipersonne < nombre_immunisés_grippe + 25; ++ipersonne )
                    {
                        population[ipersonne].estContaminé(grippe);
                    }
                }
                // Mise à jour des statistiques pour les cases de la grille :
                màjStatistique(grille, population);

                //temp_grid = grille.getStatistiques();
            //}

            //MPI_Bcast(temp_grid.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 0, scomm);
            //grille.set_m_statistiques(temp_grid);
            

            // On parcout la population pour voir qui est contaminé et qui ne l'est pas, d'abord pour la grippe puis pour l'agent pathogène
            std::size_t compteur_grippe = 0, compteur_agent = 0, mouru = 0;
            //for ( auto& personne : population )
            for (int i=(rank-1)*population.size()/(nbp-1) ; i<rank*population.size()/(nbp-1); i++)
            {
                auto& personne = population[i];
                if (personne.testContaminationGrippe(grille, contexte.interactions, grippe, agent))
                {
                    compteur_grippe ++;
                    personne.estContaminé(grippe);
                }
                if (personne.testContaminationAgent(grille, agent))
                {
                    compteur_agent ++;
                    personne.estContaminé(agent);
                }
                // On vérifie si il n'y a pas de personne qui veillissent de veillesse et on génère une nouvelle personne si c'est le cas.
                if (personne.doitMourir())
                {
                    mouru++;
                    unsigned nouvelle_graine = jours_écoulés + personne.position().x*personne.position().y;
                    personne = épidémie::Individu(nouvelle_graine, contexte.espérance_de_vie, contexte.déplacement_maximal);
                    personne.setPosition(largeur_grille, hauteur_grille);
                }
                personne.veillirDUnJour();
                personne.seDéplace(grille);
            }
            //blabla temps elapsed
            /*
            middle = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = middle-start;
            std::cout << "rank "<<rank<<". Temps calcul par etape (sans affichage) : " << elapsed_seconds.count() 
                  << std::endl;//*/
            /*MPI_Gather(
                &population[(rank-1)*population.size()/(nbp-1)],
                population.size()*sizeof(épidémie::Individu)/((nbp-1)),
                MPI_CHAR,
                &population[0],
                population.size()*sizeof(épidémie::Individu)/((nbp-1)),
                MPI_CHAR,
                0,
                scomm);*/

            MPI_Allgather(
                &population[(rank-1)*population.size()/(nbp-1)],
                population.size()*sizeof(épidémie::Individu)/((nbp-1)),
                MPI_CHAR,
                &population[0],
                population.size()*sizeof(épidémie::Individu)/((nbp-1)),
                MPI_CHAR,
                scomm);


            int local_compteur_grippe = compteur_grippe;
            int local_compteur_agent = compteur_agent;
            int local_mouru = mouru;
            MPI_Allreduce(&local_compteur_grippe, &compteur_grippe, 1, MPI_INT, MPI_SUM, scomm);
            MPI_Allreduce(&local_compteur_agent, &compteur_agent, 1, MPI_INT, MPI_SUM, scomm);
            MPI_Allreduce(&local_mouru, &mouru, 1, MPI_INT, MPI_SUM, scomm);


            //envoi des donnees
            //std::cout << "rank "<<rank<<" trying to send message" << std::endl;
            /*int MPI_Iprobe(
              int source,
              int tag,
              MPI_Comm comm,
              int *flag,
              MPI_Status *status
            );*/
            if (rank == 1){
                MPI_Iprobe( MPI_ANY_SOURCE, 4, MPI_COMM_WORLD, &ready_to_receive_flag,  MPI_STATUS_IGNORE );
                if (ready_to_receive_flag){
                    MPI_Recv(&ready_to_receive , 1, MPI_INT, MPI_ANY_SOURCE, 4, MPI_COMM_WORLD ,& status );
                
                    //MPI_Isend(&jours_écoulés, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,& jours_request ) ;
                    MPI_Send(&jours_écoulés, 1, MPI_INT, 0, 2, MPI_COMM_WORLD);
                    

                    temp_grid = grille.getStatistiques();
                    //std::cout << "3 : "<< sizeof(temp_grid) <<","<< sizeof(grille.getStatistiques()) << std::endl;
                    
                    //std::cout << sizeof(temp_grid) <<","<< contexte.taux_population <<","<< 3*largeur_grille*hauteur_grille << std::endl;
                    //MPI_Isend(temp_grid.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 0, 0, MPI_COMM_WORLD,& grille_request ) ;
                    MPI_Send(temp_grid.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 0, 3, MPI_COMM_WORLD);
                    //std::cout << "rank "<<rank<<" message sent" << std::endl;
                }
            }
            MPI_Test(&quit_request, &flag,&quit_status) ;
            if (flag!=0){
                //received
                quitting = true;// = zero_quit
                //std::cout << "rank "<<rank<<" quitting = true" << std::endl;
            
            }
        }
        //#############################################################################################################
        //##    Affichage des résultats pour le temps  actuel
        //#############################################################################################################
        /*if (affiche) {
            afficheSimulation(écran, grille, jours_écoulés);
        }*/
        /*std::cout << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
                  << grille.nombreTotalContaminésAgentPathogène() << std::endl;*/
        if (rank == 0){
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds2 = end-start;
            std::cout << "rank "<<rank<<", iteration "<<nb_iteration<<". Temps calcul par etape (recuperation + affichage ): " << elapsed_seconds2.count() 
                  << std::endl;
        }else{
            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds2 = end-start;
            std::cout << "rank "<<rank<<", iteration "<<nb_iteration<<". Temps calcul par etape (calcul + envoi ): " << elapsed_seconds2.count() 
                  << std::endl;
        }
        
        
        output << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
               << grille.nombreTotalContaminésAgentPathogène() << std::endl;
        jours_écoulés += 1;



    }// Fin boucle temporelle
    output.close();
}

int main(int argc, char* argv[])
{
    // On initialise le contexte MPI qui va s'occuper :
    //    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
    //       et assurer la cohésion de l'ensemble des processus créés par MPI;
    //    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
    //       le communicateur COMM_WORLD
    //    3. etc...
    MPI_Init( &argc, &argv );
    // Pour des raisons de portabilité qui débordent largement du cadre
    // de ce cours, on préfère toujours cloner le communicateur global
    // MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    // On interroge le communicateur global pour connaître le nombre de processus
    // qui ont été lancés par l'utilisateur :
    int nbp;
    MPI_Comm_size(globComm, &nbp);
    // On interroge le communicateur global pour connaître l'identifiant qui
    // m'a été attribué ( en tant que processus ). Cet identifiant est compris
    // entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
    // l'utilisateur )
    int rank;
    MPI_Comm_rank(globComm, &rank);

    

    // parse command-line
    bool affiche = true;
    for (int i=0; i<argc; i++) {
      std::cout << i << " " << argv[i] << "\n";
      if (std::string("-nw") == argv[i]) affiche = false;
    }
    sdl2::init(argc, argv);
    {
        simulation(affiche,rank,nbp);
    }
    sdl2::finalize();


    //std::cout << "rank "<<rank<<" reach barrier" << std::endl;
            
    MPI_Barrier(MPI_COMM_WORLD);  
    // A la fin du programme, on doit synchroniser une dernière fois tous les processus
    // afin qu'aucun processus ne se termine pendant que d'autres processus continue à
    // tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
    // qui ne seront pas encore terminés.
    //std::cout << "rank "<<rank<<" finalize" << std::endl;
    MPI_Finalize();

    return EXIT_SUCCESS;
}
