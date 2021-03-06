// Produit matrice-vecteur
# include <cassert>
# include <vector>
# include <iostream>
# include <mpi.h>

// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix (int dim);
    Matrix( int nrows, int ncols , int i_start, int j_start);
    Matrix( const Matrix& A ) = delete;
    Matrix( Matrix&& A ) = default;
    ~Matrix() = default;

    Matrix& operator = ( const Matrix& A ) = delete;
    Matrix& operator = ( Matrix&& A ) = default;
    
    double& operator () ( int i, int j ) {
        return m_arr_coefs[(i-m_i_start) + (j-m_j_start)*m_nrows];
    }
    double  operator () ( int i, int j ) const {
        return m_arr_coefs[(i-m_i_start) + (j-m_j_start)*m_nrows];
    }
    
    std::vector<double> operator * ( const std::vector<double>& u ) const;
    
    std::ostream& print( std::ostream& out ) const
    {
        const Matrix& A = *this;
        out << "[\n";
        for ( int i = 0; i < m_nrows; ++i ) {
            out << " [ ";
            for ( int j = 0; j < m_ncols; ++j ) {
                out << A(i,j) << " ";
            }
            out << " ]\n";
        }
        out << "]";
        return out;
    }
private:
    int m_nrows, m_ncols, m_i_start, m_j_start;
    std::vector<double> m_arr_coefs;
};
// ---------------------------------------------------------------------
inline std::ostream& 
operator << ( std::ostream& out, const Matrix& A )
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream&
operator << ( std::ostream& out, const std::vector<double>& u )
{
    out << "[ ";
    for ( const auto& x : u )
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double> 
Matrix::operator * ( const std::vector<double>& u ) const
{
    const Matrix& A = *this;
    assert( u.size() == unsigned(m_ncols) );
    std::vector<double> v(m_nrows, 0.);
    for ( int i = 0; i < m_nrows; ++i ) {
        for ( int j = 0; j < m_ncols; ++j ) {
            v[i] += A(i,j)*u[j];
        }            
    }
    return v;
}

// =====================================================================
Matrix::Matrix (int dim) : m_nrows(dim), m_ncols(dim),m_i_start(0),m_j_start(0),
                           m_arr_coefs(dim*dim)
{
    for ( int i = 0; i < dim; ++ i ) {
        for ( int j = 0; j < dim; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix( int nrows, int ncols, int i_start, int j_start ) : m_nrows(nrows), m_ncols(ncols),
                                         m_i_start(i_start),m_j_start(j_start), m_arr_coefs(nrows*ncols)
{
    int dim = (nrows > ncols ? nrows : ncols );
    for ( int i = i_start; i < i_start+nrows; ++ i ) {
        for ( int j = j_start; j < j_start+ncols; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }    
}
// =====================================================================

void mySum(double *invec, double *inoutvec, int *len, MPI_Datatype *dtype)
{
    int i;
    for ( i=0; i<*len; i++ ) inoutvec[i] += invec[i];
}


int main( int nargs, char* argv[] )
{
    // On initialise le contexte MPI qui va s'occuper :
	//    1. Cr??er un communicateur global, COMM_WORLD qui permet de g??rer
	//       et assurer la coh??sion de l'ensemble des processus cr????s par MPI;
	//    2. d'attribuer ?? chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init( &nargs, &argv );
	// Pour des raisons de portabilit?? qui d??bordent largement du cadre
	// de ce cours, on pr??f??re toujours cloner le communicateur global
	// MPI_COMM_WORLD qui g??re l'ensemble des processus lanc??s par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour conna??tre le nombre de processus
	// qui ont ??t?? lanc??s par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour conna??tre l'identifiant qui
	// m'a ??t?? attribu?? ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp ??tant le nombre de processus qui ont ??t?? lanc??s par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);

	//MPI_Status status ;

    const int N = 12;
    const int Nloc = N/nbp;
    
    MPI_Op vSum;
    MPI_Op_create( (MPI_User_function *)mySum, 1, &vSum );

/*
    //test
    MPI_Datatype myVector;
    MPI_Type_vector(N, 1, 1, MPI_DOUBLE, &myVector);
    MPI_Type_commit(&myVector);
*/    
    
    
    
    std::vector<double> u( N );
    for ( int i = 0; i < N; ++i ) u[i] = i+1;
    //std::cout << " u : " << u << std::endl;
    //std::vector<double> v = A*u;
    //std::cout << "A.u = " << v << std::endl;


/*
    //Produit parall??le matrice ??? vecteur par colonne
    Matrix A(N,Nloc, 0, Nloc*rank);
    //std::cout  << "A : " << A << std::endl;

    std::vector<double> v_part( N );
    std::vector<double> v_final( N );
    for (int i = 0 ; i < N ; i++){
        v_part[i] = 0;
        for (int j = Nloc*rank ; j < Nloc*(rank+1) ; j++){
            v_part[i] += A(i,j)*u[j];
        }
    }
    MPI_Allreduce (&v_part[0], &v_final[0], N, MPI_DOUBLE, vSum, MPI_COMM_WORLD);
    std::cout << "v = A*u : " << v_final << " ( je suis le processus n??" << rank << ".)\n";
//*/
///*
    //Produit parall??le matrice ??? vecteur par ligne
    Matrix A(Nloc,N, Nloc*rank, 0);
    //std::cout  << "A : " << A << std::endl;

    std::vector<double> v_part( Nloc);
    std::vector<double> v_final( N );
    for (int i = 0 ; i < Nloc ; i++){
        v_part[i] = 0;
        for (int j = 0 ; j < N ; j++){
            v_part[i] += A(i+Nloc*rank,j)*u[j];
        }
    }
    MPI_Allgather (&v_part[0], Nloc, MPI_DOUBLE, &v_final[0], Nloc, MPI_DOUBLE, MPI_COMM_WORLD );
    //MPI_Allgather( sendarray, 100, MPI_INT, rbuf, 100, MPI_INT, comm); 
    std::cout << "v = A*u : " << v_final << " ( je suis le processus n??" << rank << ".)\n";
//*/

    MPI_Finalize();
    return EXIT_SUCCESS;
}
