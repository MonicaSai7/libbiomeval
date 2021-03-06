/**
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties.  Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain.  NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 */
#ifndef _BE_MPI_WORKPACKAGEPROCESSOR_H
#define _BE_MPI_WORKPACKAGEPROCESSOR_H

#include <memory>
#include <be_io_logsheet.h>
#include <be_mpi_workpackage.h>

/**
 * An interface to the object that processes a package of work from
 * the MPI Receiver.
 */
namespace BiometricEvaluation {
	namespace MPI {

		/**
		 * @brief
		 * Represents an object that processes the contents of a
		 * work package.
		 * @details
		 * A WorkPackageProcessor presents two personalities: One that
		 * of a worker to process work packages, and one that is a
		 * factory to return  worker objects of the implementation
		 * class.
		 *
		 * Subclasses of this class implement the functionality needed
		 * to perform an action on the work package data. The processing
		 * done by the implementation is application and data type
		 * specific.
		 *
		 * Ultimately, the final implementation of the
		 * WorkPackageProcessor class is done in the application.
		 * Access to the Logsheet object maintained by the framework
		 * is provided by this class.
		 */
		class WorkPackageProcessor {
		public:
			/**
			 * @brief
			 * Obtain an object that will process work packages.
			 * This method is part of the factory personality.
			 * @param logsheet
			 * A shared pointer to the IO::Logsheet that may be
			 * used to save messages generated by the object.
			 * @return
			 * A shared pointer to the work package processor.
			 * @note
			 * This method should always create a non-null
			 * WorkPackageProcessor. If an error occurs during
			 * construction, throw a Error::Exception with a
			 * message to be caught and logged.
			 */
			virtual std::shared_ptr<WorkPackageProcessor>
			    newProcessor(
				std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			/**
			 * @brief
			 * Initialization function to be called before work
			 * is distributed to the work package processor.
			 * @details
			 * Implementations of this class can use this function
			 * to do any processing necessary before work is
			 * given to the processor, pre-forking.
			 *
			 * This method is part of the factory personality.
			 * All state that is to be common across all package
			 * processor objects can be initialized in this method.
			 *
			 * @param logsheet
			 * A shared pointer to the IO::Logsheet that may be
			 * used to save messages generated by the object.
			 * @throw Error::Exception
			 * An implementation specific error occurred. The
			 * exception string will be logged by the Framework.
			 */
			virtual void performInitialization(
				std::shared_ptr<IO::Logsheet> &logsheet) = 0;

			/**
			 * @brief
			 * Process the data contents of the work package.
			 * This method is part of the worker personality.
			 * @param[in] workPackage
			 * The work package.
			 * @throw Error::Exception
			 * An fatal error occurred when processing the work
			 * package; the processing responsible for this
			 * object should shut down.
			 */
			virtual void processWorkPackage(
			    MPI::WorkPackage &workPackage) = 0;

			/**
			 * @brief
			 * Terminiation function to be called during shut down
			 * after all work package processing is done.
			 * @details
			 * Implementations of this class can use this function
			 * to do any processing necessary after all work is
			 * given to the processors. The default implementation
			 * does nothing.
			 *
			 * This method is part of the factory personality.
			 * All state that is created in performInitialization()
			 * processor objects can be accessed in this method.
			 *
			 * @throw Error::Exception
			 * An implementation specific error occurred. The
			 * exception string will be logged by the Framework.
			 */
			virtual void performShutdown();

			/**
			 * @brief
			 * Set the IO::Logsheet object that can be used
			 * to save message for objects of this class.
			 * @param[in] logsheet
			 * A shared pointer to the Logsheet object.
			 */
			void
			setLogsheet(std::shared_ptr<IO::Logsheet> &logsheet);

			/**
			 * @brief
			 * Obtain the IO::Logsheet object that can be used
			 * to save message for objects of this class.
			 * @return logsheet
			 * A shared pointer to the Logsheet object.
			 */
			std::shared_ptr<IO::Logsheet> getLogsheet();

			virtual ~WorkPackageProcessor();

		protected:
		private:
			std::shared_ptr<IO::Logsheet> _logsheet;
		};
	}
}

#endif /* _BE_MPI_WORKPACKAGEPROCESSOR_H */

